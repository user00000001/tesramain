// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "miner.h"

#include "amount.h"
#include "hash.h"
#include "main.h"
#include "masternode-sync.h"
#include "net.h"
#include "pow.h"
#include "primitives/block.h"
#include "script/sign.h"
#include "timedata.h"
#include "util.h"
#include "utilmoneystr.h"
#ifdef ENABLE_WALLET
#include "wallet.h"
#endif
#include "masternode-payments.h"
#include "accumulators.h"
#include "spork.h"

#include <boost/thread.hpp>
#include <boost/tuple/tuple.hpp>

using namespace std;

//////////////////////////////////////////////////////////////////////////////
//
// TESRAMiner
//

//
// Unconfirmed transactions in the memory pool often depend on other
// transactions in the memory pool. When we select transactions from the
// pool, we select by highest priority or fee rate, so we might consider
// transactions that depend on transactions that aren't yet in the block.
// The COrphan class keeps track of these 'temporary orphans' while
// CreateBlock is figuring out which transactions to include.
//


///////////////////////////////////////////// // -vm
    CMutableTransaction originalRewardTx;

    ByteCodeExecResult bceResult;
    uint64_t minGasPrice = 1;
    uint64_t hardBlockGasLimit;
    uint64_t softBlockGasLimit;
    uint64_t txGasLimit;
/////////////////////////////////////////////




class COrphan
{
public:
    const CTransaction* ptx;
    set<uint256> setDependsOn;
    CFeeRate feeRate;
    double dPriority;

    COrphan(const CTransaction* ptxIn) : ptx(ptxIn), feeRate(0), dPriority(0)
    {
    }
};

uint64_t nLastBlockTx = 0;
uint64_t nLastBlockSize = 0;
int64_t nLastCoinStakeSearchInterval = 0;

// We want to sort transactions by priority and fee rate, so:
typedef boost::tuple<double, CFeeRate, const CTransaction*> TxPriority;
class TxPriorityCompare
{
    bool byFee;

public:
    TxPriorityCompare(bool _byFee) : byFee(_byFee) {}

    bool operator()(const TxPriority& a, const TxPriority& b)
    {
        if (byFee) {
            if (a.get<1>() == b.get<1>())
                return a.get<0>() < b.get<0>();
            return a.get<1>() < b.get<1>();
        } else {
            if (a.get<0>() == b.get<0>())
                return a.get<1>() < b.get<1>();
            return a.get<0>() < b.get<0>();
        }
    }
};

void UpdateTime(CBlockHeader* pblock, const CBlockIndex* pindexPrev)
{
    pblock->nTime = std::max(pindexPrev->GetMedianTimePast() + 1, GetAdjustedTime());

    // Updating time can change work required on testnet:
    if (Params().AllowMinDifficultyBlocks()) {
        pblock->nBits = GetNextWorkRequired(pindexPrev, pblock);
#ifdef  POW_IN_POS_PHASE
        if (pindexPrev->nHeight >= Params().POW_Start_BLOCK_In_POS() - 1) {
            pblock->nBits2 = GetNextPowWorkRequired(pindexPrev, pblock);
        }
#endif
    }
}

CBlockTemplate* CreateNewPowBlock(CBlockIndex* pindexPrev, CWallet* pwallet)
{
    CPubKey pubkey;
    CReserveKey reservekey(pwallet);
    
    if (!reservekey.GetReservedKey(pubkey))
        return NULL;

    CScript scriptPubKey = CScript() << ToByteVector(pubkey) << OP_CHECKSIG;

    // Create new block
    unique_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate());
    if (!pblocktemplate.get())
        return NULL;

    if (!ReadBlockFromDisk(pblocktemplate->block, pindexPrev))
        return NULL;
    
    CBlock* pblock = &pblocktemplate->block; // pointer for convenience

    if (!pblock->IsProofOfStake())
        return NULL;

    CAmount nTxFees = 0;

    // Create coinbase tx
    CMutableTransaction txNew;
    txNew.vin.resize(1);
    txNew.vin[0].prevout.SetNull();
    txNew.vout.resize(1);
    txNew.vout[0].scriptPubKey = scriptPubKey;
    
    pblock->vtx[0] = txNew;

    //  POW block coinbase don't contain masternode payment in POS phase.
    pblock->payee.clear();

    int nBlockSigOps = 100;

    pblocktemplate->vTxFees.push_back(-1);
//    pblocktemplate->vTxSigOps.push_back(-1);
    {
        LOCK2(cs_main, mempool.cs);

        CCoinsViewCache view(pcoinsTip);

        int index = 0;
        for (const CTransaction tx : pblock->vtx) {
            unsigned int nMaxBlockSigOps = MAX_BLOCK_SIGOPS_CURRENT;
            unsigned int nTxSigOps = GetLegacySigOpCount(tx);
            if (nBlockSigOps + nTxSigOps >= nMaxBlockSigOps)
                continue;

            nTxSigOps += GetP2SHSigOpCount(tx, view);
            if (nBlockSigOps + nTxSigOps >= nMaxBlockSigOps)
                continue;

            pblocktemplate->vTxFees[index] = nTxFees;
        }
    }

    return pblocktemplate.release();
}


void RebuildRefundTransaction(CBlock *pblock,CAmount &nFees)
{
    CMutableTransaction contrTx(originalRewardTx);

    contrTx.vout[2].nValue -= bceResult.refundSender;

    //note, this will need changed for MPoS
    int i = contrTx.vout.size();
    contrTx.vout.resize(contrTx.vout.size() + bceResult.refundOutputs.size());
    for (CTxOut &vout : bceResult.refundOutputs)
    {
        contrTx.vout[i] = vout;
        i++;
    }
    pblock->vtx[1] = std::move(contrTx);

}


bool AttemptToAddContractToBlock(const CTransaction &iter, uint64_t minGasPrice,CBlockTemplate *pblockTemplate,uint64_t &nBlockSize,int &nBlockSigOps,uint64_t &nBlockTx,CCoinsViewCache &view,CAmount &nFees)
{

    //FixMe: Check if this cause something unexpected.
//    if (nTimeLimit != 0 && GetAdjustedTime() >= nTimeLimit - BYTECODE_TIME_BUFFER) {
//        return false;
//    }
    if (GetBoolArg("-disablecontractstaking", false))
    {
        return false;
    }

    uint256 oldHashStateRoot;
    uint256 oldHashUTXORoot;

    GetState(oldHashStateRoot, oldHashUTXORoot);

    // operate on local vars first, then later apply to `this`
    uint64_t nBlockWeight = nBlockSize;
    int nBlockSigOpsCost = nBlockSigOps;

    CBlock *pblock = &pblockTemplate->block;

    TesraTxConverter convert(iter, NULL, &pblock->vtx);

    ExtractTesraTX resultConverter;
    if(!convert.extractionTesraTransactions(resultConverter)){
        //this check already happens when accepting txs into mempool
        //therefore, this can only be triggered by using raw transactions on the staker itself
        return false;
    }
    std::vector<TesraTransaction> tesraTransactions = resultConverter.first;
    dev::u256 txGas = 0;
    for(TesraTransaction tesraTransaction : tesraTransactions){
        txGas += tesraTransaction.gas();
        if(txGas > txGasLimit) {
            // Limit the tx gas limit by the soft limit if such a limit has been specified.
            return false;
        }

        if(bceResult.usedGas + tesraTransaction.gas() > softBlockGasLimit){
            //if this transaction's gasLimit could cause block gas limit to be exceeded, then don't add it
            return false;
        }
        if(tesraTransaction.gasPrice() < minGasPrice){
            //if this transaction's gasPrice is less than the current DGP minGasPrice don't add it
            return false;
        }
    }
    // We need to pass the DGP's block gas limit (not the soft limit) since it is consensus critical.
    ByteCodeExec exec(*pblock, tesraTransactions, hardBlockGasLimit);
    if(!exec.performByteCode()){
        //error, don't add contract
        UpdateState(oldHashStateRoot,oldHashUTXORoot);
        return false;
    }



    ByteCodeExecResult testExecResult;
    if(!exec.processingResults(testExecResult)){
        UpdateState(oldHashStateRoot,oldHashUTXORoot);
        return false;
    }


    if(bceResult.usedGas + testExecResult.usedGas > softBlockGasLimit){
        //if this transaction could cause block gas limit to be exceeded, then don't add it
        UpdateState(oldHashStateRoot,oldHashUTXORoot);

        return false;
    }


    //apply contractTx costs to local state
    nBlockSize += ::GetSerializeSize(iter, SER_NETWORK, PROTOCOL_VERSION);
    nBlockSigOpsCost += GetLegacySigOpCount(iter);
    //apply value-transfer txs to local state


    for (CTransaction &t : testExecResult.valueTransfers) {
        nBlockWeight += ::GetSerializeSize(t, SER_NETWORK, PROTOCOL_VERSION);;
        nBlockSigOpsCost += GetLegacySigOpCount(t);
    }


    int proofTx = pblock->IsProofOfStake() ? 1 : 0;

    //calculate sigops from new refund/proof tx

    //first, subtract old proof tx
    nBlockSigOpsCost -= GetLegacySigOpCount(pblock->vtx[proofTx]);



    // manually rebuild refundtx
    CMutableTransaction contrTx(pblock->vtx[proofTx]);
    //note, this will need changed for MPoS
    int i = contrTx.vout.size();
    contrTx.vout.resize(contrTx.vout.size()+testExecResult.refundOutputs.size());
    for(CTxOut& vout : testExecResult.refundOutputs){
        contrTx.vout[i]=vout;
        i++;
    }
    nBlockSigOpsCost += GetLegacySigOpCount(contrTx);
    //all contract costs now applied to local state


    //Check if block will be too big or too expensive with this contract execution
    if (nBlockSigOpsCost > (int)MAX_BLOCK_SIGOPS_CURRENT ||
            nBlockWeight > GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE)) {
        //contract will not be added to block, so revert state to before we tried
        UpdateState(oldHashStateRoot,oldHashUTXORoot);

        return false;
    }

    //block is not too big, so apply the contract execution and it's results to the actual block

    //apply local bytecode to global bytecode state
    bceResult.usedGas += testExecResult.usedGas;
    bceResult.refundSender += testExecResult.refundSender;
    bceResult.refundOutputs.insert(bceResult.refundOutputs.end(), testExecResult.refundOutputs.begin(), testExecResult.refundOutputs.end());
    bceResult.valueTransfers = std::move(testExecResult.valueTransfers);


    pblock->vtx.emplace_back(iter);


    //CAmount in_value = view.GetValueIn(iter);


    pblockTemplate->vTxFees.push_back(view.GetValueIn(iter) - iter.GetValueOut());

    pblockTemplate->vTxSigOps.push_back(GetLegacySigOpCount(iter));

    nBlockSize += ::GetSerializeSize(iter, SER_NETWORK, PROTOCOL_VERSION);
    ++nBlockTx;
    nBlockSigOps += GetLegacySigOpCount(iter);

    nFees += view.GetValueIn(iter) - iter.GetValueOut();



    for (CTransaction &t : bceResult.valueTransfers) {
        pblock->vtx.emplace_back(t);
        nBlockSize += ::GetSerializeSize(t, SER_NETWORK, PROTOCOL_VERSION);
        nBlockSigOps += GetLegacySigOpCount(t);
        ++nBlockTx;
    }
    //calculate sigops from new refund/proof tx
    nBlockSigOps -= GetLegacySigOpCount(pblock->vtx[proofTx]);

    RebuildRefundTransaction(pblock,nFees);

    nBlockSigOps += GetLegacySigOpCount(pblock->vtx[proofTx]);

    bceResult.valueTransfers.clear();

    return true;
}

CBlockTemplate* CreateNewBlock(const CScript& scriptPubKeyIn, CWallet* pwallet, bool fProofOfStake)
{
    CReserveKey reservekey(pwallet);

    // Create new block
    //Init global vars //-----tesra-vm
    originalRewardTx.nLockTime = 0;
    originalRewardTx.nVersion = 0;
    originalRewardTx.vin.clear();
    originalRewardTx.vout.clear();

    bceResult.refundSender = 0;
    bceResult.usedGas = 0;
    bceResult.refundOutputs.clear();
    bceResult.valueTransfers.clear();

    minGasPrice = 1;
    hardBlockGasLimit = 0;
    softBlockGasLimit = 0;
    txGasLimit = 0;
    //-----tesra-vm

    unique_ptr<CBlockTemplate> pblocktemplate(new CBlockTemplate());
    if (!pblocktemplate.get())
        return NULL;
    CBlock* pblock = &pblocktemplate->block; // pointer for convenience

    // -regtest only: allow overriding block.nVersion with
    // -blockversion=N to test forking scenarios
    if (Params().MineBlocksOnDemand())
        pblock->nVersion = GetArg("-blockversion", pblock->nVersion);

    // Make sure to create the correct block version after zerocoin is enabled
    bool fZerocoinActive = (GetAdjustedTime() >= Params().Zerocoin_StartTime()) && (chainActive.Height() + 1 >= Params().Zerocoin_StartHeight());
    bool fContractActive = (chainActive.Height() + 1 >= Params().Contract_StartHeight());
    if (chainActive.Height() + 1 <= Params().LAST_POW_BLOCK()) {
        pblock->nVersion = POW_VERSION;
    } else if (!fZerocoinActive)
        pblock->nVersion = POS_VERSION;
    else if (!fContractActive)
        pblock->nVersion = ZEROCOIN_VERSION;
    else
        pblock->nVersion = SMART_CONTRACT_VERSION;

    // Create coinbase tx
    CMutableTransaction txNew;
    CMutableTransaction txCoinStake;
   // CMutableTransaction coinbaseTxBak; //tesra-vm

    txNew.vin.resize(1);
    txNew.vin[0].prevout.SetNull();
    txNew.vout.resize(1);
    txNew.vout[0].scriptPubKey = scriptPubKeyIn;
    pblock->vtx.push_back(txNew);
    pblocktemplate->vTxFees.push_back(-1);   // updated at end
    pblocktemplate->vTxSigOps.push_back(-1); // updated at end
    //coinbaseTxBak = txNew; //tesra-vm

    // ppcoin: if coinstake available add coinstake tx
    static int64_t nLastCoinStakeSearchTime = GetAdjustedTime(); // only initialized at startup

    if (fProofOfStake) {
        boost::this_thread::interruption_point();
        pblock->nTime = GetAdjustedTime();
        CBlockIndex* pindexPrev = chainActive.Tip();
        pblock->nBits = GetNextWorkRequired(pindexPrev, pblock);
#ifdef  POW_IN_POS_PHASE
        pblock->nBits2 = GetNextPowWorkRequired(pindexPrev, pblock);
#endif
        int64_t nSearchTime = pblock->nTime; // search to current time
        bool fStakeFound = false;
        if (nSearchTime >= nLastCoinStakeSearchTime) {
            unsigned int nTxNewTime = 0;
            if (pwallet->CreateCoinStake(*pwallet, pblock, nSearchTime - nLastCoinStakeSearchTime, txCoinStake, nTxNewTime)) {
                pblock->nTime = nTxNewTime;
                pblock->vtx[0].vout[0].SetEmpty();
                pblock->vtx.push_back(CTransaction(txCoinStake));
                originalRewardTx = txCoinStake;
                fStakeFound = true;
            }
            nLastCoinStakeSearchInterval = nSearchTime - nLastCoinStakeSearchTime;
            nLastCoinStakeSearchTime = nSearchTime;
        }

        if (!fStakeFound)
            return NULL;
    }

    //////////////////////////////////////////////////////// tesra-vm
      minGasPrice = GetMinGasPrice(chainActive.Height()+1);
      if (GetBoolArg("-staker-min-tx-gas-price", false))
      {
          CAmount stakerMinGasPrice;
          string strMinxGasPrice = "";
          strMinxGasPrice = GetArg("-staker-min-tx-gas-price", strMinxGasPrice);
          if (ParseMoney(strMinxGasPrice, stakerMinGasPrice))
          {
              minGasPrice = std::max(minGasPrice, (uint64_t)stakerMinGasPrice);
          }
      }
      hardBlockGasLimit = GetBlockGasLimit(chainActive.Height()+1);
      softBlockGasLimit = GetArg("-staker-soft-block-gas-limit", hardBlockGasLimit);
      softBlockGasLimit = std::min(softBlockGasLimit, hardBlockGasLimit);
      txGasLimit = GetArg("-staker-max-tx-gas-limit", softBlockGasLimit);

      //    nBlockMaxSize = blockSizeDGP ? blockSizeDGP : nBlockMaxSize;

      uint256 oldHashStateRoot, oldHashUTXORoot;
      GetState(oldHashStateRoot, oldHashUTXORoot);


      ////////////////////////////////////////////////////////










    // Largest block you're willing to create:
    unsigned int nBlockMaxSize = GetArg("-blockmaxsize", DEFAULT_BLOCK_MAX_SIZE);
    // Limit to betweeen 1K and MAX_BLOCK_SIZE-1K for sanity:
    unsigned int nBlockMaxSizeNetwork = MAX_BLOCK_SIZE_CURRENT;
    nBlockMaxSize = std::max((unsigned int)1000, std::min((nBlockMaxSizeNetwork - 1000), nBlockMaxSize));

    // How much of the block should be dedicated to high-priority transactions,
    // included regardless of the fees they pay
    unsigned int nBlockPrioritySize = GetArg("-blockprioritysize", DEFAULT_BLOCK_PRIORITY_SIZE);
    nBlockPrioritySize = std::min(nBlockMaxSize, nBlockPrioritySize);

    // Minimum block size you want to create; block will be filled with free transactions
    // until there are no more or the block reaches this size:
    unsigned int nBlockMinSize = GetArg("-blockminsize", DEFAULT_BLOCK_MIN_SIZE);
    nBlockMinSize = std::min(nBlockMaxSize, nBlockMinSize);

    // Collect memory pool transactions into the block
    CAmount nFees = 0;

    {
        LOCK2(cs_main, mempool.cs);

        CBlockIndex* pindexPrev = chainActive.Tip();
        const int nHeight = pindexPrev->nHeight + 1;
        CCoinsViewCache view(pcoinsTip);

        // Priority order to process transactions
        list<COrphan> vOrphan; // list memory doesn't move
        map<uint256, vector<COrphan*> > mapDependers;
        bool fPrintPriority = GetBoolArg("-printpriority", false);

        // This vector will be sorted into a priority queue:
        vector<TxPriority> vecPriority;
        vecPriority.reserve(mempool.mapTx.size());
        for (map<uint256, CTxMemPoolEntry>::iterator mi = mempool.mapTx.begin();
             mi != mempool.mapTx.end(); ++mi) {
            const CTransaction& tx = mi->second.GetTx();
            if (tx.IsCoinBase() || tx.IsCoinStake() || !IsFinalTx(tx, nHeight)){
                continue;
            }

            if (GetAdjustedTime() > GetSporkValue(SPORK_16_ZEROCOIN_MAINTENANCE_MODE) && tx.ContainsZerocoins())
                continue;

            COrphan* porphan = NULL;
            double dPriority = 0;
            CAmount nTotalIn = 0;
            bool fMissingInputs = false;
            for (const CTxIn& txin : tx.vin) {
                //zerocoinspend has special vin
                if (tx.IsZerocoinSpend()) {
                    nTotalIn = tx.GetZerocoinSpent();
                    break;
                }

                // Read prev transaction
                if (!view.HaveCoins(txin.prevout.hash)) {
                    // This should never happen; all transactions in the memory
                    // pool should connect to either transactions in the chain
                    // or other transactions in the memory pool.
                    if (!mempool.mapTx.count(txin.prevout.hash)) {
                        LogPrintf("ERROR: mempool transaction missing input\n");
                        if (fDebug) assert("mempool transaction missing input" == 0);
                        fMissingInputs = true;
                        if (porphan)
                            vOrphan.pop_back();
                        break;
                    }

                    // Has to wait for dependencies
                    if (!porphan) {
                        // Use list for automatic deletion
                        vOrphan.push_back(COrphan(&tx));
                        porphan = &vOrphan.back();
                    }
                    mapDependers[txin.prevout.hash].push_back(porphan);
                    porphan->setDependsOn.insert(txin.prevout.hash);
                    nTotalIn += mempool.mapTx[txin.prevout.hash].GetTx().vout[txin.prevout.n].nValue;
                    continue;
                }

                //Check for invalid/fraudulent inputs. They shouldn't make it through mempool, but check anyways.
                if (mapInvalidOutPoints.count(txin.prevout)) {
                    LogPrintf("%s : found invalid input %s in tx %s", __func__, txin.prevout.ToString(), tx.GetHash().ToString());
                    fMissingInputs = true;
                    break;
                }

                const CCoins* coins = view.AccessCoins(txin.prevout.hash);
                assert(coins);

                CAmount nValueIn = coins->vout[txin.prevout.n].nValue;
                nTotalIn += nValueIn;

                int nConf = nHeight - coins->nHeight;

                dPriority += (double)nValueIn * nConf;
            }
            if (fMissingInputs) continue;

            // Priority is sum(valuein * age) / modified_txsize
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            dPriority = tx.ComputePriority(dPriority, nTxSize);

            uint256 hash = tx.GetHash();
            mempool.ApplyDeltas(hash, dPriority, nTotalIn);

            CFeeRate feeRate(nTotalIn - tx.GetValueOut(), nTxSize);

            if (porphan) {
                porphan->dPriority = dPriority;
                porphan->feeRate = feeRate;
            } else
                vecPriority.push_back(TxPriority(dPriority, feeRate, &mi->second.GetTx()));
        }

        // Collect transactions into block
        uint64_t nBlockSize = 1000;
        uint64_t nBlockTx = 0;
        int nBlockSigOps = 100;
        bool fSortedByFee = (nBlockPrioritySize <= 0);

        TxPriorityCompare comparer(fSortedByFee);
        std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);

        vector<CBigNum> vBlockSerials;
        vector<CBigNum> vTxSerials;

        //-------------tesra-vm
        while (!vecPriority.empty()) {



            // Take highest priority transaction off the priority queue:
            double dPriority = vecPriority.front().get<0>();
            CFeeRate feeRate = vecPriority.front().get<1>();
            const CTransaction& tx = *(vecPriority.front().get<2>());

            std::pop_heap(vecPriority.begin(), vecPriority.end(), comparer);
            vecPriority.pop_back();

            // Size limits
            unsigned int nTxSize = ::GetSerializeSize(tx, SER_NETWORK, PROTOCOL_VERSION);
            if (nBlockSize + nTxSize >= nBlockMaxSize)
                continue;

            // Legacy limits on sigOps:
            unsigned int nMaxBlockSigOps = MAX_BLOCK_SIGOPS_CURRENT;
            unsigned int nTxSigOps = GetLegacySigOpCount(tx);
            if (nBlockSigOps + nTxSigOps >= nMaxBlockSigOps)
                continue;

            // Skip free transactions if we're past the minimum block size:
            const uint256& hash = tx.GetHash();
            double dPriorityDelta = 0;
            CAmount nFeeDelta = 0;
            mempool.ApplyDeltas(hash, dPriorityDelta, nFeeDelta);
            if (!tx.IsZerocoinSpend() && fSortedByFee && (dPriorityDelta <= 0) && (nFeeDelta <= 0) && (feeRate < ::minRelayTxFee) && (nBlockSize + nTxSize >= nBlockMinSize))
                continue;

            // Prioritise by fee once past the priority size or we run out of high-priority
            // transactions:
            if (!fSortedByFee &&
                ((nBlockSize + nTxSize >= nBlockPrioritySize) || !AllowFree(dPriority))) {
                fSortedByFee = true;
                comparer = TxPriorityCompare(fSortedByFee);
                std::make_heap(vecPriority.begin(), vecPriority.end(), comparer);
            }

            if (!view.HaveInputs(tx))
                continue;

            // double check that there are no double spent zTsr spends in this block or tx
            if (tx.IsZerocoinSpend()) {
                int nHeightTx = 0;
                if (IsTransactionInChain(tx.GetHash(), nHeightTx))
                    continue;

                bool fDoubleSerial = false;
                for (const CTxIn txIn : tx.vin) {
                    if (txIn.scriptSig.IsZerocoinSpend()) {
                        libzerocoin::CoinSpend spend = TxInToZerocoinSpend(txIn);
                        if (!spend.HasValidSerial(Params().Zerocoin_Params()))
                            fDoubleSerial = true;
                        if (count(vBlockSerials.begin(), vBlockSerials.end(), spend.getCoinSerialNumber()))
                            fDoubleSerial = true;
                        if (count(vTxSerials.begin(), vTxSerials.end(), spend.getCoinSerialNumber()))
                            fDoubleSerial = true;
                        if (fDoubleSerial)
                            break;
                        vTxSerials.emplace_back(spend.getCoinSerialNumber());
                    }
                }
                //This zTsr serial has already been included in the block, do not add this tx.
                if (fDoubleSerial)
                    continue;
            }

            CAmount nTxFees = view.GetValueIn(tx) - tx.GetValueOut();

            nTxSigOps += GetP2SHSigOpCount(tx, view);
            if (nBlockSigOps + nTxSigOps >= nMaxBlockSigOps)
                continue;

            // Note that flags: we don't want to set mempool/IsStandard()
            // policy here, but we still have to ensure that the block we
            // create only contains transactions that are valid in new blocks.
            CValidationState state;
            if (!CheckInputs(tx, state, view, true, MANDATORY_SCRIPT_VERIFY_FLAGS, true))
                continue;


            // Added
            //--------tesra-vm---
            if (tx.HasCreateOrCall()) {
                if(!AttemptToAddContractToBlock(tx, minGasPrice,pblocktemplate.get(),nBlockSize,nBlockSigOps,nBlockTx,view, nFees))
                {
                    std::pop_heap(vecPriority.begin(), vecPriority.end(), comparer);
                    vecPriority.pop_back();
                    continue;
                }

            } else {
                pblock->vtx.push_back(tx);
                pblocktemplate->vTxFees.push_back(nTxFees);
                pblocktemplate->vTxSigOps.push_back(nTxSigOps);
                nBlockSize += nTxSize;
                ++nBlockTx;
                nBlockSigOps += nTxSigOps;
                nFees += nTxFees;


                if (fPrintPriority) {
                    LogPrintf("priority %.1f fee %s txid %s\n",
                        dPriority, feeRate.ToString(), tx.GetHash().ToString());
                }

            }
            for (const CBigNum bnSerial : vTxSerials)
                vBlockSerials.emplace_back(bnSerial);

            //--------tesra-vm---

            CTxUndo txundo;
            UpdateCoins(tx, state, view, txundo, nHeight);



            // Add transactions that depend on this one to the priority queue
            if (mapDependers.count(hash)) {
                BOOST_FOREACH (COrphan* porphan, mapDependers[hash]) {
                    if (!porphan->setDependsOn.empty()) {
                        porphan->setDependsOn.erase(hash);
                        if (porphan->setDependsOn.empty()) {
                            vecPriority.push_back(TxPriority(porphan->dPriority, porphan->feeRate, porphan->ptx));
                            std::push_heap(vecPriority.begin(), vecPriority.end(), comparer);
                        }
                    }
                }
            }
        }

        if (!fProofOfStake) {
            //Masternode and general budget payments
            FillBlockPayee(txNew, GetBlockValue(pindexPrev->nHeight), nFees, fProofOfStake);

            //Make payee
            if (txNew.vout.size() > 1) {
                pblock->payee = txNew.vout[1].scriptPubKey;
            }
        } else {


            //--------------tesra-vm---------------
            //this should already be populated by AddBlock in case of contracts, but if no contracts
            //then it won't get populated
            if(chainActive.Tip()->nHeight >= Params().Contract_StartHeight()) {

                uint256 hashStateRoot, hashUTXORoot;
                GetState(hashStateRoot, hashUTXORoot);

                LogPrintf("CreateNewBlock: Populate state and utxo before RebuildRefundTransaction:\nstateroot:%s\nutxoRoot: %s\n", hashStateRoot.GetHex().c_str(), hashUTXORoot.GetHex().c_str());
                CScript contract = CScript() << ParseHex(hashStateRoot.GetHex().c_str()) << ParseHex(hashUTXORoot.GetHex().c_str()) << OP_VM_STATE;

                UpdateState(oldHashStateRoot, oldHashUTXORoot);

                RebuildRefundTransaction(pblock,nFees);
                txCoinStake = pblock->vtx[1];

                txCoinStake.vout[1] = CTxOut(0, contract);

            }


            //  fees to POS Miner.
            if (pblock->nVersion < SMART_CONTRACT_VERSION)
                txCoinStake.vout[1].nValue += nFees;
            else
                txCoinStake.vout[2].nValue += nFees;



            // Sign
            for (size_t nIn = 0; nIn < txCoinStake.vin.size(); nIn++) {
                const CWalletTx* pcoin = pwallet->GetWalletTx(txCoinStake.vin[nIn].prevout.hash);

                if (!SignSignature(*pwallet, *pcoin, txCoinStake, nIn)) {
                    LogPrintStr("CreateCoinStake : failed to re-sign coinstake");
                    return NULL;
                }
            }

            pblock->vtx[1] = CTransaction(txCoinStake);
        }

        nLastBlockTx = nBlockTx;
        nLastBlockSize = nBlockSize;
        LogPrintf("CreateNewBlock(): total size %u\n", nBlockSize);







        //--------------tesra-vm---------------

        // Compute final coinbase transaction.
        if (!fProofOfStake) {
            pblock->vtx[0] = txNew;
            pblocktemplate->vTxFees[0] = -nFees;
        }
        pblock->vtx[0].vin[0].scriptSig = CScript() << nHeight << OP_0;

        // Fill in header
        pblock->hashPrevBlock = pindexPrev->GetBlockHash();
        if (!fProofOfStake)
            UpdateTime(pblock, pindexPrev);
        pblock->nBits = GetNextWorkRequired(pindexPrev, pblock);
        if (!fProofOfStake)
            pblock->nNonce = 0;
        uint256 nCheckpoint = 0;
        if(fZerocoinActive && !CalculateAccumulatorCheckpoint(nHeight, nCheckpoint)){
            LogPrintf("%s: failed to get accumulator checkpoint\n", __func__);
        }
        pblock->nAccumulatorCheckpoint = nCheckpoint;
        pblocktemplate->vTxSigOps[0] = GetLegacySigOpCount(pblock->vtx[0]);

        CValidationState state;
        if (!TestBlockValidity(state, *pblock, pindexPrev, false, false)) {
            LogPrintf("CreateNewBlock() : TestBlockValidity failed\n");
            mempool.clear();
            return NULL;
        }
    }

    return pblocktemplate.release();
}

void IncrementExtraNonce(CBlock* pblock, CBlockIndex* pindexPrev, unsigned int& nExtraNonce)
{
    // Update nExtraNonce
    static uint256 hashPrevBlock;
    if (hashPrevBlock != pblock->hashPrevBlock) {
        nExtraNonce = 0;
        hashPrevBlock = pblock->hashPrevBlock;
    }
    ++nExtraNonce;
    unsigned int nHeight = pindexPrev->nHeight + 1; // Height first in coinbase required for block.version=2
    CMutableTransaction txCoinbase(pblock->vtx[0]);
    txCoinbase.vin[0].scriptSig = (CScript() << nHeight << CScriptNum(nExtraNonce)) + COINBASE_FLAGS;
    assert(txCoinbase.vin[0].scriptSig.size() <= 100);

    pblock->vtx[0] = txCoinbase;
    pblock->hashMerkleRoot = pblock->BuildMerkleTree();
}

#ifdef ENABLE_WALLET
//////////////////////////////////////////////////////////////////////////////
//
// Internal miner
//
double dHashesPerSec = 0.0;
int64_t nHPSTimerStart = 0;

CBlockTemplate* CreateNewBlockWithKey(CReserveKey& reservekey, CWallet* pwallet, bool fProofOfStake)
{
    CPubKey pubkey;
    if (!reservekey.GetReservedKey(pubkey))
        return NULL;

    CScript scriptPubKey = CScript() << ToByteVector(pubkey) << OP_CHECKSIG;

	if(newBlockAddress.IsValid())
		scriptPubKey = GetScriptForDestination(newBlockAddress.Get());

    return CreateNewBlock(scriptPubKey, pwallet, fProofOfStake);
}

bool ProcessBlockFound(CBlock* pblock, CWallet& wallet, CReserveKey& reservekey)
{
    LogPrintf("%s\n", pblock->ToString());
    LogPrintf("generated %s\n", FormatMoney(pblock->vtx[0].vout[0].nValue));

    // Found a solution
    {
        LOCK(cs_main);
        if (pblock->hashPrevBlock != chainActive.Tip()->GetBlockHash())
            return error("TESRAMiner : generated block is stale");
    }

    // Remove key from key pool
    reservekey.KeepKey();

    // Track how many getdata requests this block gets
    {
        LOCK(wallet.cs_wallet);
        wallet.mapRequestCount[pblock->GetHash()] = 0;
    }

    // Process this block the same as if we had received it from another node
    CValidationState state;
    if (!ProcessNewBlock(state, NULL, pblock))
        return error("TESRAMiner : ProcessNewBlock, block not accepted");

    for (CNode* node : vNodes) {
        node->PushInventory(CInv(MSG_BLOCK, pblock->GetHash()));
    }

    return true;
}

bool fGenerateBitcoins = false;

// ***TODO*** that part changed in bitcoin, we are using a mix with old one here for now

void BitcoinMiner(CWallet* pwallet, bool fProofOfStake)
{
    LogPrintf("TESRA %s Miner started\n", fProofOfStake ? "POS" : "POW");
    SetThreadPriority(THREAD_PRIORITY_LOWEST);
    RenameThread("tesra-miner");

    // Each thread has its own key and counter
    CReserveKey reservekey(pwallet);
    unsigned int nExtraNonce = 0;

    //control the amount of times the client will check for mintable coins
    static bool fMintableCoins = false;
    static int nMintableLastCheck = 0;


    while (fGenerateBitcoins || fProofOfStake) {
        if (chainActive.Height() < Params().LAST_POW_BLOCK() || fProofOfStake) {
            if (fProofOfStake) {
                if ((GetTime() - nMintableLastCheck > 5 * 60)) // 5 minute check time
                {
                    nMintableLastCheck = GetTime();
                    fMintableCoins = pwallet->MintableCoins();
                }

                if (chainActive.Tip()->nHeight < Params().LAST_POW_BLOCK()) {
                    MilliSleep(5000);
                    continue;
                }

                while (vNodes.empty() || pwallet->IsLocked() || !fMintableCoins || (pwallet->GetBalance() > 0 && nReserveBalance >= pwallet->GetBalance()) || !masternodeSync.IsSynced()) {
                    nLastCoinStakeSearchInterval = 0;
                    // Do a separate 1 minute check here to ensure fMintableCoins is updated
                    if (!fMintableCoins) {
                        if (GetTime() - nMintableLastCheck > 1 * 60) // 1 minute check time
                        {
                            nMintableLastCheck = GetTime();
                            fMintableCoins = pwallet->MintableCoins();
                        }
                    }
                    MilliSleep(5000);
                    if (!fGenerateBitcoins && !fProofOfStake)
                        continue;
                }

                if (mapHashedBlocks.count(chainActive.Tip()->nHeight)) //search our map of hashed blocks, see if bestblock has been hashed yet
                {
                    if (GetTime() - mapHashedBlocks[chainActive.Tip()->nHeight] < max(pwallet->nHashInterval, (unsigned int)1)) // wait half of the nHashDrift with max wait of 3 minutes
                    {
                        MilliSleep(5000);
                        continue;
                    }
                }
            }

            //
            // Create new block
            //
            unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();
            CBlockIndex* pindexPrev = chainActive.Tip();
            if (!pindexPrev)
                continue;

            unique_ptr<CBlockTemplate> pblocktemplate(CreateNewBlockWithKey(reservekey, pwallet, fProofOfStake));
            if (!pblocktemplate.get())
                continue;

            CBlock* pblock = &pblocktemplate->block;
            IncrementExtraNonce(pblock, pindexPrev, nExtraNonce);

            //Stake miner main
            if (fProofOfStake) {
                LogPrintf("CPU POS Miner : proof-of-stake block found %s \n", pblock->GetHash().ToString().c_str());

                if (!pblock->SignBlock(*pwallet)) {
                    LogPrintf("BitcoinMiner(): Signing new block failed \n");
                    continue;
                }

                LogPrintf("CPUMiner : proof-of-stake block was signed %s \n", pblock->GetHash().ToString().c_str());
                SetThreadPriority(THREAD_PRIORITY_NORMAL);
                ProcessBlockFound(pblock, *pwallet, reservekey);
                SetThreadPriority(THREAD_PRIORITY_LOWEST);

                continue;
            }

            LogPrintf("Running TESRAMiner with %u transactions in block (%u bytes)\n", pblock->vtx.size(),
                ::GetSerializeSize(*pblock, SER_NETWORK, PROTOCOL_VERSION));

            //
            // Search
            //
            int64_t nStart = GetTime();
            uint256 hashTarget = uint256().SetCompact(pblock->nBits);
            while (true) {
                unsigned int nHashesDone = 0;

                uint256 hash;
                while (true) {
                    hash = pblock->GetPoWHash();
                    if (hash <= hashTarget) {
                        // Found a solution
                        SetThreadPriority(THREAD_PRIORITY_NORMAL);
                        LogPrintf("BitcoinMiner:\n");
                        LogPrintf("proof-of-work found  \n  hash: %s  \ntarget: %s\n", hash.GetHex(), hashTarget.GetHex());
                        ProcessBlockFound(pblock, *pwallet, reservekey);
                        SetThreadPriority(THREAD_PRIORITY_LOWEST);

                        // In regression test mode, stop mining after a block is found. This
                        // allows developers to controllably generate a block on demand.
                        if (Params().MineBlocksOnDemand())
                            throw boost::thread_interrupted();

                        break;
                    }
                    pblock->nNonce += 1;
                    nHashesDone += 1;
                    if ((pblock->nNonce & 0xFF) == 0)
                        break;
                }

                // Meter hashes/sec
                static int64_t nHashCounter;
                if (nHPSTimerStart == 0) {
                    nHPSTimerStart = GetTimeMillis();
                    nHashCounter = 0;
                } else
                    nHashCounter += nHashesDone;
                if (GetTimeMillis() - nHPSTimerStart > 4000) {
                    static CCriticalSection cs;
                    {
                        LOCK(cs);
                        if (GetTimeMillis() - nHPSTimerStart > 4000) {
                            dHashesPerSec = 1000.0 * nHashCounter / (GetTimeMillis() - nHPSTimerStart);
                            nHPSTimerStart = GetTimeMillis();
                            nHashCounter = 0;
                            static int64_t nLogTime;
                            if (GetTime() - nLogTime > 30 * 60) {
                                nLogTime = GetTime();
                                LogPrintf("hashmeter %6.0f khash/s\n", dHashesPerSec / 1000.0);
                            }
                        }
                    }
                }

                // Check for stop or if block needs to be rebuilt
                boost::this_thread::interruption_point();
                // Regtest mode doesn't require peers
                if (vNodes.empty() && Params().MiningRequiresPeers())
                    break;
                if (pblock->nNonce >= 0xffff0000)
                    break;
                if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
                    break;
                if (pindexPrev != chainActive.Tip())
                    break;

                // Update nTime every few seconds
                UpdateTime(pblock, pindexPrev);
                if (Params().AllowMinDifficultyBlocks()) {
                    // Changing pblock->nTime can change work required on testnet:
                    hashTarget.SetCompact(pblock->nBits);
                }
            }
        } else {
            CBlock block;

            CPubKey pubkey;

            static int height = 0;

            CBlockIndex *pindexCurrent = chainActive.Tip();

            //  In pos phase, pow miner wait the height.
            if (pindexCurrent->nHeight < Params().POW_Start_BLOCK_In_POS() - 1) {
                MilliSleep(5000);
                continue;
            }

#ifdef      POW_IN_POS_PHASE
            if (height != pindexCurrent->nHeight) {
                if(ReadBlockFromDisk(block, pindexCurrent) && block.IsProofOfStake() && reservekey.GetReservedKey(pubkey)) {
                    height = pindexCurrent->nHeight;
                    
                    CMutableTransaction coinBaseTx;

                    CScript scriptPubKey = CScript() << ToByteVector(pubkey) << OP_CHECKSIG;

                    coinBaseTx.vin.resize(1);
                    coinBaseTx.vin[0].prevout.SetNull();
                    coinBaseTx.vout.resize(1);
                    coinBaseTx.vout[0].scriptPubKey = scriptPubKey;
                    coinBaseTx.vout[0].nValue = GetTmpBlockValue(height);

                    block.vtx[0] = coinBaseTx;
                    block.hashMerkleRoot = block.BuildMerkleTree();

                    block.nNonce = 0;
                    block.nBits = block.nBits2;

                    int64_t nStart = GetTime();
                    unsigned int nTransactionsUpdatedLast = mempool.GetTransactionsUpdated();

                    uint256 blockhash = *pindexCurrent->phashBlock;
                    uint256 hashTarget = uint256().SetCompact(block.nBits);
                    while (true) {
                        unsigned int nHashesDone = 0;

                        uint256 hash;
                        while (pindexCurrent == chainActive.Tip()) {
                            hash = block.GetPoWHash();
//                            hash = HashCryptoNight(BEGIN(block.nVersion), END(block.nAccumulatorCheckpoint));
                            if (hash < hashTarget) {
                                CTmpBlockParams tmpBlockParams;

                                tmpBlockParams.ori_hash = blockhash;
                                tmpBlockParams.nNonce = block.nNonce;
                                tmpBlockParams.coinBaseTx = coinBaseTx;

                                CBlockHeader blockHeader = block.GetBlockHeader();

                                ProcessNewTmpBlockParam(tmpBlockParams, blockHeader);

                                // Remove key from key pool
                                reservekey.KeepKey();
                            }
                            block.nNonce += 1;
                            nHashesDone += 1;
                            if ((block.nNonce & 0xFF) == 0)
                                break;
                        }

                        // Meter hashes/sec
                        static int64_t nHashCounter;
                        if (nHPSTimerStart == 0) {
                            nHPSTimerStart = GetTimeMillis();
                            nHashCounter = 0;
                        } else
                            nHashCounter += nHashesDone;
                        if (GetTimeMillis() - nHPSTimerStart > 4000) {
                            static CCriticalSection cs;
                            {
                                LOCK(cs);
                                if (GetTimeMillis() - nHPSTimerStart > 4000) {
                                    dHashesPerSec = 1000.0 * nHashCounter / (GetTimeMillis() - nHPSTimerStart);
                                    nHPSTimerStart = GetTimeMillis();
                                    nHashCounter = 0;
                                    static int64_t nLogTime;
                                    if (GetTime() - nLogTime > 30 * 60) {
                                        nLogTime = GetTime();
                                        LogPrintf("hashmeter %6.0f khash/s\n", dHashesPerSec / 1000.0);
                                    }
                                }
                            }
                        }
                        
                        // Check for stop or if block needs to be rebuilt
                        boost::this_thread::interruption_point();
                        // Regtest mode doesn't require peers
                        if (vNodes.empty() && Params().MiningRequiresPeers())
                            break;
                        if (block.nNonce >= 0xffff0000)
                            break;
                        if (mempool.GetTransactionsUpdated() != nTransactionsUpdatedLast && GetTime() - nStart > 60)
                            break;
                        if (pindexCurrent != chainActive.Tip())
                            break;
                    }
                } else {
                    MilliSleep(1000);
                }
            } else {
#endif
                MilliSleep(5000);
#ifdef      POW_IN_POS_PHASE
            }
#endif
        }
    }
}

void static ThreadBitcoinMiner(void* parg)
{
    boost::this_thread::interruption_point();
    CWallet* pwallet = (CWallet*)parg;
    try {
        BitcoinMiner(pwallet, false);
        boost::this_thread::interruption_point();
    } catch (std::exception& e) {
        LogPrintf("ThreadBitcoinMiner() exception");
    } catch (...) {
        LogPrintf("ThreadBitcoinMiner() exception");
    }

    LogPrintf("ThreadBitcoinMiner exiting\n");
}

void GenerateBitcoins(bool fGenerate, CWallet* pwallet, int nThreads)
{
    static boost::thread_group* minerThreads = NULL;
    fGenerateBitcoins = fGenerate;

    if (nThreads < 0) {
        // In regtest threads defaults to 1
        if (Params().DefaultMinerThreads())
            nThreads = Params().DefaultMinerThreads();
        else
            nThreads = boost::thread::hardware_concurrency();
    }

    if (minerThreads != NULL) {
        minerThreads->interrupt_all();
        delete minerThreads;
        minerThreads = NULL;
    }

    if (nThreads == 0 || !fGenerate)
        return;

    minerThreads = new boost::thread_group();
    for (int i = 0; i < nThreads; i++)
        minerThreads->create_thread(boost::bind(&ThreadBitcoinMiner, pwallet));
}

#endif // ENABLE_WALLET
