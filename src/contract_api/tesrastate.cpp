#include <map>
#include <sstream>
#include <util.h>
#include "main.h"
#include "chainparams.h"
#include "tesrastate.h"
#include "extendeddata.h"
#include "utilstrencodings.h"
#include "contractbase.h"
#include "libethereum/Transaction.h"
using namespace std;
using namespace dev;
using namespace dev::eth;

#define HDR_LEN_SIZE    (2)
#define HDR_TYPE_SIZE   (1)
#define HDR_KEY_SIZE    (32)

static const size_t MAX_CONTRACT_VOUTS = 1000;

TesraState::TesraState(u256 const &_accountStartNonce, OverlayDB const &_db, const string &_path, BaseState _bs) :
        State(_accountStartNonce, _db, _bs)
{
    dbUTXO = TesraState::openDB(_path + "/tesraDB", sha3(rlp("")), WithExisting::Trust);
    stateUTXO = SecureTrieDB<Address, OverlayDB>(&dbUTXO);
}

TesraState::TesraState() : dev::eth::State(dev::Invalid256, dev::OverlayDB(), dev::eth::BaseState::PreExisting)
{
    dbUTXO = OverlayDB();
    stateUTXO = SecureTrieDB<Address, OverlayDB>(&dbUTXO);
}

ResultExecute
TesraState::execute(EnvInfo const &_envInfo, SealEngineFace const &_sealEngine, TesraTransaction const &_t, Permanence _p,
                   OnOpFunc const &_onOp)
{

    assert(_t.getVersion().toRaw() == VersionVM::GetEVMDefault().toRaw());

    addBalance(_t.sender(), _t.value() + (_t.gas() * _t.gasPrice()));
    newAddress = _t.isCreation() ? createTesraAddress(_t.getHashWith(), _t.getNVout()) : dev::Address();

    //LogPrintf("TesraState::execute sender=%s\n", HexStr(_t.sender().asBytes())); //tesra debug
    //LogPrintf("TesraState::execute newAddress=%s\n", HexStr(newAddress.asBytes())); //tesra debug
   // LogPrintf("TesraState::execute author=%s\n", HexStr(_envInfo.author().asBytes())); //tesra debug
    _sealEngine.deleteAddresses.insert({_t.sender(), _envInfo.author()});

    h256 oldStateRoot = rootHash();
    bool voutLimit = false;

    auto onOp = _onOp;
#if ETH_VMTRACE
    if (isChannelVisible<VMTraceChannel>())
        onOp = Executive::simpleTrace(); // override tracer
#endif
    // Create and initialize the executive. This will throw fairly cheaply and quickly if the
    // transaction is bad in any way.
    Executive e(*this, _envInfo, _sealEngine);
    ExecutionResult res;
    e.setResultRecipient(res);

    CTransaction tx;
    u256 startGasUsed;
    try
    {
        if (_t.isCreation() && _t.value())
            BOOST_THROW_EXCEPTION(CreateWithValue());
       // LogPrintStr("TesraState::initialize\n"); //tesra debug
        e.initialize(_t);
        // OK - transaction looks valid - execute.
        startGasUsed = _envInfo.gasUsed();
        if (!e.execute())
        {
            e.go(onOp);
        } else
        {

            e.revert();
            throw Exception();
        }
        e.finalize();
        if (_p == Permanence::Reverted)
        {
            m_cache.clear();
            cacheUTXO.clear();
        } else
        {
            //LogPrintStr("TesraState::execute ok\n"); //tesra debug
            deleteAccounts(_sealEngine.deleteAddresses);
            if (res.excepted == TransactionException::None)
            {
                CondensingTX ctx(this, transfers, _t, _sealEngine.deleteAddresses);
                tx = ctx.createCondensingTX();  //tesra call contract tx =>output from/to value
                if (ctx.reachedVoutLimit())
                {
                   // LogPrintStr("TesraState::execute voutOverflow = 1\n"); //tesra debug
                    voutLimit = true;
                    e.revert();
                    throw Exception();
                }
                std::unordered_map<dev::Address, Vin> vins = ctx.createVin(tx);

                updateUTXO(vins);
            } else
            {
                printfErrorLog(res.excepted);
            }

          //  LogPrintStr("TesraState::before execute commit\n"); //tesra debug
            tesra::commit(cacheUTXO, stateUTXO, m_cache);
            cacheUTXO.clear();
          //  LogPrintStr("TesraState::after execute commit\n"); //tesra debug

            //  FixMe: force removeEmptyAccounts false, is used to control EmptyAccounts in vmstate.
            //  bool removeEmptyAccounts = _envInfo.number() >= _sealEngine.chainParams().u256Param("EIP158ForkBlock");
            bool removeEmptyAccounts = false;
            commit(removeEmptyAccounts ? State::CommitBehaviour::RemoveEmptyAccounts
                                       : State::CommitBehaviour::KeepEmptyAccounts);

          //  LogPrintStr("TesraState::final commit\n"); //tesra debug

        }
    }
    catch (Exception const &_e)
    {
      //  LogPrintStr("Exception::something wrong\n"); //tesra debug

        printfErrorLog(dev::eth::toTransactionException(_e));
        res.excepted = dev::eth::toTransactionException(_e);
        res.gasUsed = _t.gas();
//        const Consensus::Params &consensusParams = Params().GetConsensus();
        //tesra-vm force to clear
        //        if(chainActive.Height() < consensusParams.nFixUTXOCacheHFHeight  && _p != Permanence::Reverted){
        //            deleteAccounts(_sealEngine.deleteAddresses);
        //            commit(CommitBehaviour::RemoveEmptyAccounts);
        //        } else
        {
            m_cache.clear();
            cacheUTXO.clear();
        }
    }
    //LogPrintStr("TesraState::before judge isCreation\n"); //tesra debug

    if (!_t.isCreation())
        res.newAddress = _t.receiveAddress();

   // LogPrintStr("TesraState::after judge isCreation\n"); //tesra debug

    newAddress = dev::Address();

    //LogPrintStr("TesraState::after assign newAddress\n"); //tesra debug

    transfers.clear();
    //LogPrintStr("TesraState::after transfers clear\n"); //tesra debug

    if (voutLimit)
    {
      //  LogPrintStr("TesraState:: in voutLimit judge\n"); //tesra debug

        //use old and empty states to create virtual Out Of Gas exception
        LogEntries logs;
        u256 gas = _t.gas();
        ExecutionResult ex;
        ex.gasRefunded = 0;
        ex.gasUsed = gas;
        ex.excepted = TransactionException();
        //create a refund tx to send back any coins that were suppose to be sent to the contract
        CMutableTransaction refund;
        if (_t.value() > 0)
        {
            refund.vin.push_back(CTxIn(h256Touint(_t.getHashWith()), _t.getNVout(), CScript() << OP_SPEND));
            //note, if sender was a non-standard tx, this will send the coins to pubkeyhash 0x00, effectively destroying the coins
            CScript script(CScript() << OP_DUP << OP_HASH160 << _t.sender().asBytes() << OP_EQUALVERIFY << OP_CHECKSIG);
            refund.vout.push_back(CTxOut(CAmount(_t.value().convert_to<uint64_t>()), script));
        }
        //make sure to use empty transaction if no vouts made
      //  LogPrintStr("TesraState::execute voutLimit true"); //tesra debug
        return ResultExecute{ex, dev::eth::TransactionReceipt(oldStateRoot, gas, e.logs()),
                             refund.vout.empty() ? CTransaction() : CTransaction(refund)};
    } else
    {
       // LogPrintStr("before:: in else voutLimit judge ResultExecute\n"); //tesra debug

        return ResultExecute{res, dev::eth::TransactionReceipt(rootHash(), startGasUsed + e.gasUsed(), e.logs()),
                             tx};
    }
}

std::unordered_map<dev::Address, Vin> TesraState::vins() const // temp
{
    std::unordered_map<dev::Address, Vin> ret;
    for (auto &i: cacheUTXO)
        if (i.second.alive)
            ret[i.first] = i.second;
    auto addrs = addresses();
    for (auto &i : addrs)
    {
        if (cacheUTXO.find(i.first) == cacheUTXO.end() && vin(i.first))
            ret[i.first] = *vin(i.first);
    }
    return ret;
}

void TesraState::transferBalance(dev::Address const &_from, dev::Address const &_to, dev::u256 const &_value)
{
    subBalance(_from, _value);
    addBalance(_to, _value);
    if (_value > 0)
    {
        LogPrint("TesraState::transferBalance ", "_from=%s", HexStr(_from.asBytes())); //tesra debug
        LogPrint("TesraState::transferBalance ", "_to=%s", HexStr(_to.asBytes())); //tesra debug
        LogPrint("TesraState::transferBalance ", "_value=%s", _value.str(0, std::ios_base::hex).c_str()); //tesra debug
        transfers.push_back({_from, _to, _value});
    }
}

Vin const *TesraState::vin(dev::Address const &_a) const
{
    return const_cast<TesraState *>(this)->vin(_a);
}

Vin *TesraState::vin(dev::Address const &_addr)
{
    auto it = cacheUTXO.find(_addr);
    if (it == cacheUTXO.end())
    {
        std::string stateBack = stateUTXO.at(_addr);
        LogPrint("TesraState::stateBack ", "%s", stateBack); //tesra debug
        if (stateBack.empty())
            return nullptr;

        dev::RLP state(stateBack);
        auto i = cacheUTXO.emplace(
                std::piecewise_construct,
                std::forward_as_tuple(_addr),
                std::forward_as_tuple(
                        Vin{state[0].toHash<dev::h256>(), state[1].toInt<uint32_t>(), state[2].toInt<dev::u256>(),
                            state[3].toInt<uint8_t>()})
        );
       // LogPrintStr("TesraState::stateBack construct"); //tesra debug
        return &i.first->second;
    }
  //  LogPrintStr("TesraState::vin find"); //tesra debug
    return &it->second;
}

// void TesraState::commit(CommitBehaviour _commitBehaviour)
// {
//     if (_commitBehaviour == CommitBehaviour::RemoveEmptyAccounts)
//         removeEmptyAccounts();

//     tesra::commit(cacheUTXO, stateUTXO, m_cache);
//     cacheUTXO.clear();

//     m_touched += dev::eth::commit(m_cache, m_state);
//     m_changeLog.clear();
//     m_cache.clear();
//     m_unchangedCacheEntries.clear();
// }

void TesraState::kill(dev::Address _addr)
{
    // If the account is not in the db, nothing to kill.
    if (auto a = account(_addr))
        a->kill();
    if (auto v = vin(_addr))
        v->alive = 0;
}

void TesraState::addBalance(dev::Address const &_id, dev::u256 const &_amount)
{
    if (dev::eth::Account *a = account(_id))
    {
        // Log empty account being touched. Empty touched accounts are cleared
        // after the transaction, so this event must be also reverted.
        // We only log the first touch (not dirty yet), and only for empty
        // accounts, as other accounts does not matter.
        // TODO: to save space we can combine this event with Balance by having
        //       Balance and Balance+Touch events.
        if (!a->isDirty() && a->isEmpty())
            m_changeLog.emplace_back(dev::eth::Change::Touch, _id);

        // Increase the account balance. This also is done for value 0 to mark
        // the account as dirty. Dirty account are not removed from the cache
        // and are cleared if empty at the end of the transaction.
        a->addBalance(_amount);
    } else
    {
        if (!addressInUse(newAddress) && newAddress != dev::Address())
        {
            const_cast<dev::Address &>(_id) = newAddress;
            newAddress = dev::Address();
        }
        createAccount(_id, {requireAccountStartNonce(), _amount});
    }

    if (_amount)
        m_changeLog.emplace_back(dev::eth::Change::Balance, _id, _amount);
}

dev::Address TesraState::createTesraAddress(dev::h256 hashTx, uint32_t voutNumber)
{
    uint256 hashTXid(h256Touint(hashTx));
    std::vector<unsigned char> txIdAndVout(hashTXid.begin(), hashTXid.end());
    std::vector<unsigned char> voutNumberChrs;
    if (voutNumberChrs.size() < sizeof(voutNumber))
        voutNumberChrs.resize(sizeof(voutNumber));
    std::memcpy(voutNumberChrs.data(), &voutNumber, sizeof(voutNumber));
    txIdAndVout.insert(txIdAndVout.end(), voutNumberChrs.begin(), voutNumberChrs.end());

    std::vector<unsigned char> SHA256TxVout(32);
    CSHA256().Write(txIdAndVout.data(), txIdAndVout.size()).Finalize(SHA256TxVout.data());

    std::vector<unsigned char> hashTxIdAndVout(20);
    CRIPEMD160().Write(SHA256TxVout.data(), SHA256TxVout.size()).Finalize(hashTxIdAndVout.data());

    return dev::Address(hashTxIdAndVout);
}

void TesraState::deleteAccounts(std::set<dev::Address> &addrs)
{
    for (dev::Address addr : addrs)
    {
        dev::eth::Account *acc = const_cast<dev::eth::Account *>(account(addr));
        if (acc)
            acc->kill();
        Vin *in = const_cast<Vin *>(vin(addr));
        if (in)
            in->alive = 0;
    }
}

//tesra debug
void logVin(string str, Vin vin)
{
    LogPrint("", "%s", str); //tesra debug
    LogPrint("", "hash=%s", vin.hash.hex()); //tesra debug
    LogPrint("", "nVout=%d", vin.nVout); //tesra debug
    LogPrint("", "value=%s", vin.value.str(0, std::ios_base::hex).c_str()); //tesra debug
    LogPrint("", "alive=%d", vin.alive); //tesra debug
}

void TesraState::updateUTXO(const std::unordered_map<dev::Address, Vin> &vins)
{
    for (auto &v : vins)
    {
        Vin *vi = const_cast<Vin *>(vin(v.first));

        LogPrint("TesraState::", "updateUTXO inaddress=%s", v.first.hex()); //tesra debug
        logVin("updateUTXO invin", v.second);//tesra debug
        if (vi)
        {
            logVin("TesraState::updateUTXO 00", *vi);//tesra debug
            vi->hash = v.second.hash;
            vi->nVout = v.second.nVout;
            vi->value = v.second.value;
            vi->alive = v.second.alive;
            logVin("TesraState::updateUTXO 11", *vi);//tesra debug
        } else if (v.second.alive > 0)
        {
            LogPrint("TesraState::", "updateUTXO 22"); //tesra debug
            cacheUTXO[v.first] = v.second;
        }
    }
}

void TesraState::printfErrorLog(const dev::eth::TransactionException er)
{
    std::stringstream ss;
    ss << er;
    LogPrintf("TesraState::printfErrorLog  VM exception: %s\n", ss.str().c_str());

}

///////////////////////////////////////////////////////////////////////////////////////////
CTransaction CondensingTX::createCondensingTX()
{
    selectionVin();
    calculatePlusAndMinus();
    if (!createNewBalances())
        return CTransaction();
    CMutableTransaction tx;
    tx.vin = createVins();;
    tx.vout = createVout();
    // tesra debug print vin,vout
    return !tx.vin.size() || !tx.vout.size() ? CTransaction() : CTransaction(tx);
}

std::unordered_map<dev::Address, Vin> CondensingTX::createVin(const CTransaction &tx)
{
    std::unordered_map<dev::Address, Vin> vins;
    for (auto &b : balances)
    {
        if (b.first == transaction.sender())
            continue;

        if (b.second > 0)
        {
            vins[b.first] = Vin{uintToh256(tx.GetHash()), nVouts[b.first], b.second, 1};
        } else
        {
            vins[b.first] = Vin{uintToh256(tx.GetHash()), 0, 0, 0};
        }
    }
    return vins;
}

void CondensingTX::selectionVin()
{
    for (const TransferInfo &ti : transfers)
    {
        if (!vins.count(ti.from))
        {
            LogPrint("CondensingTX::selectionVin ", "from=%s", HexStr(ti.from.asBytes())); //tesra debug
            if (auto a = state->vin(ti.from))
            {
                vins[ti.from] = *a;
                logVin("CondensingTX::selectionVin0", *a);//tesra debug
            }
            if (ti.from == transaction.sender() && transaction.value() > 0)
            {
                vins[ti.from] = Vin{transaction.getHashWith(), transaction.getNVout(), transaction.value(), 1};
                logVin("CondensingTX::selectionVin1", vins[ti.from]);//tesra debug
            }
        }
        if (!vins.count(ti.to))
        {
            LogPrint("CondensingTX::selectionVin ", "to=%s", HexStr(ti.to.asBytes())); //tesra debug
            if (auto a = state->vin(ti.to))
            {
                vins[ti.to] = *a;
                logVin("CondensingTX::selectionVin2", *a);//tesra debug
            }
        }
    }
}

void CondensingTX::calculatePlusAndMinus()
{
    for (const TransferInfo &ti : transfers)
    {
        LogPrint("CondensingTX::calculatePlusAndMinus ", "from=%s", HexStr(ti.from.asBytes())); //tesra debug
        if (!plusMinusInfo.count(ti.from))
        {
            plusMinusInfo[ti.from] = std::make_pair(0, ti.value);
            LogPrint("CondensingTX::calculatePlusAndMinus ", "first= %d,second value=%s", 0, ti.value.str(0, std::ios_base::hex).c_str()); //tesra debug
        } else
        {
            plusMinusInfo[ti.from] = std::make_pair(plusMinusInfo[ti.from].first,
                                                    plusMinusInfo[ti.from].second + ti.value);
            LogPrint("CondensingTX::calculatePlusAndMinus ", "first= %s,second value=%s", plusMinusInfo[ti.from].first.str(0, std::ios_base::hex).c_str(),
                       plusMinusInfo[ti.from].second.str(0, std::ios_base::hex).c_str()); //tesra debug
        }

        LogPrint("CondensingTX::calculatePlusAndMinus ", "to=%s", HexStr(ti.to.asBytes())); //tesra debug
        if (!plusMinusInfo.count(ti.to))
        {
            plusMinusInfo[ti.to] = std::make_pair(ti.value, 0);
            LogPrint("CondensingTX::calculatePlusAndMinus ", "first value= %s,second=%d", ti.value.str(0, std::ios_base::hex).c_str(), 0); //tesra debug
        } else
        {
            plusMinusInfo[ti.to] = std::make_pair(plusMinusInfo[ti.to].first + ti.value, plusMinusInfo[ti.to].second);
            LogPrint("CondensingTX::calculatePlusAndMinus ", "first value= %s,second=%s", plusMinusInfo[ti.to].first.str(0, std::ios_base::hex).c_str(),
                       plusMinusInfo[ti.to].second.str(0, std::ios_base::hex).c_str()); //tesra debug
        }
    }
}

bool CondensingTX::createNewBalances()
{
    for (auto &p : plusMinusInfo)
    {
        dev::u256 balance = 0;
        if ((vins.count(p.first) && vins[p.first].alive) || (!vins[p.first].alive && !checkDeleteAddress(p.first)))
        {
            balance = vins[p.first].value;
        }
        balance += p.second.first;

        if (balance < p.second.second)
            return false;
        balance -= p.second.second;
        balances[p.first] = balance;

    }
    return true;
}

std::vector<CTxIn> CondensingTX::createVins()
{
    std::vector<CTxIn> ins;
    LogPrint("CondensingTX::createVins",""); //tesra debug
    for (auto &v : vins)
    {
        if ((v.second.value > 0 && v.second.alive) ||
            (v.second.value > 0 && !vins[v.first].alive && !checkDeleteAddress(v.first)))
        {
            LogPrint("CondensingTX::createVins", "hash=%s", v.second.hash.hex()); //tesra debug
            LogPrint("CondensingTX::createVins", "nVout=%d", v.second.nVout); //tesra debug
            ins.push_back(CTxIn(h256Touint(v.second.hash), v.second.nVout, CScript() << OP_SPEND));
        }
    }
    return ins;
}

std::vector<CTxOut> CondensingTX::createVout()
{
    size_t count = 0;
    std::vector<CTxOut> outs;
    for (auto &b : balances)
    {
        if (b.second > 0)
        {
            CScript script;
            auto *a = state->account(b.first);
            if (a && a->isAlive())
            {
                //create a no-exec contract output
                script = CScript() << valtype{0} << valtype{0} << valtype{0} << valtype{0} << b.first.asBytes()
                                   << OP_CALL;
            } else
            {
                script = CScript() << OP_DUP << OP_HASH160 << b.first.asBytes() << OP_EQUALVERIFY << OP_CHECKSIG;
            }
            outs.push_back(CTxOut(CAmount(b.second), script));
            nVouts[b.first] = count;
            count++;
        }
        if (count > MAX_CONTRACT_VOUTS)
        {
            voutOverflow = true;
            return outs;
        }
    }
    return outs;
}

bool CondensingTX::checkDeleteAddress(dev::Address addr)
{
    return deleteAddresses.count(addr) != 0;
}
///////////////////////////////////////////////////////////////////////////////////////////
