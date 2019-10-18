// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2013 The Bitcoin developers
// Copyright (c) 2015-2017 The PIVX developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_PRIMITIVES_BLOCK_H
#define BITCOIN_PRIMITIVES_BLOCK_H

#include "primitives/transaction.h"
#include "keystore.h"
#include "serialize.h"
#include "uint256.h"

/** The maximum allowed size for a serialized block, in bytes (network rule) */
static const unsigned int MAX_BLOCK_SIZE_CURRENT = 32000000;
static const unsigned int MAX_BLOCK_SIZE_LEGACY = 1000000;

/** Nodes collect new transactions into a block, hash them into a hash tree,
 * and scan through nonce values to make the block's hash satisfy proof-of-work
 * requirements.  When they solve the proof-of-work, they broadcast the block
 * to everyone and the block is added to the block chain.  The first transaction
 * in the block is a special one that creates a new coin owned by the creator
 * of the block.
 */

#ifdef  POW_IN_POS_PHASE
class CTmpBlockParams
{
public:
    uint256 ori_hash;
    unsigned int nNonce;
    CTransaction coinBaseTx;
    uint256 blockheader_hash;

    CTmpBlockParams()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {

        READWRITE(ori_hash);
        READWRITE(nNonce);
        READWRITE(coinBaseTx);
    }

    uint256 GetHash() const;

    void SetNull()
    {
        blockheader_hash= 0;
        ori_hash = 0;
        nNonce = 0;
    }
};
#endif

enum VM_STATE_ROOT
{
    RET_VM_STATE_OK = 1,
    RET_CONTRACT_UNENBALE = 0,
    RET_VM_STATE_ERR = -1,
};

enum BLOCK_VERSION
{
    GENESIS_VERSION = 1,
    POW_VERSION = 2,
    POS_VERSION = 3,
    ZEROCOIN_VERSION = 4,
    SMART_CONTRACT_VERSION = 5,
};

class CBlockHeader
{
public:
    // header
    static const int32_t CURRENT_VERSION = SMART_CONTRACT_VERSION;
    int32_t nVersion;
    uint256 hashPrevBlock;
    uint256 hashMerkleRoot;
    uint32_t nTime;
    uint32_t nBits;
    uint32_t nNonce;
    uint256 nAccumulatorCheckpoint;
#ifdef  POW_IN_POS_PHASE
    uint32_t nBits2;  //For POW in POS phase
#endif

    CBlockHeader()
    {
        SetNull();
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(this->nVersion);
        nVersion = this->nVersion;
        READWRITE(hashPrevBlock);
        READWRITE(hashMerkleRoot);
        READWRITE(nTime);
        READWRITE(nBits);
        READWRITE(nNonce);

        //zerocoin active, header changes to include accumulator checksum
        if(nVersion >= ZEROCOIN_VERSION)
            READWRITE(nAccumulatorCheckpoint);

#ifdef  POW_IN_POS_PHASE
        if(nVersion >= POS_VERSION)
            READWRITE(nBits2);
#endif
    }

    void SetNull()
    {
        nVersion = CBlockHeader::CURRENT_VERSION;
        hashPrevBlock.SetNull();
        hashMerkleRoot.SetNull();
        nTime = 0;
        nBits = 0;
        nNonce = 0;
        nAccumulatorCheckpoint = 0;
#ifdef  POW_IN_POS_PHASE
        nBits2 = 0;
#endif
    }

    bool IsNull() const
    {
        return (nBits == 0);
    }

#ifdef  POW_IN_POS_PHASE
    bool IsNull2() const
    {
        return (nBits2 == 0);
    }
#endif

    uint256 GetHash() const;

    uint256 GetPoWHash() const;
    
    int64_t GetBlockTime() const
    {
        return (int64_t)nTime;
    }
};


class CBlock : public CBlockHeader
{
public:
    // network and disk
    std::vector<CTransaction> vtx;

    // ppcoin: block signature - signed by one of the coin base txout[N]'s owner
    std::vector<unsigned char> vchBlockSig;

    // memory only
    mutable CScript payee;
    mutable std::vector<uint256> vMerkleTree;

    CBlock()
    {
        SetNull();
    }

    CBlock(const CBlockHeader &header)
    {
        SetNull();
        *((CBlockHeader*)this) = header;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        READWRITE(*(CBlockHeader*)this);
        READWRITE(vtx);
	if(vtx.size() > 1 && vtx[1].IsCoinStake())
		READWRITE(vchBlockSig);
    }

    void SetNull()
    {
        CBlockHeader::SetNull();
        vtx.clear();
        vMerkleTree.clear();
        payee = CScript();
        vchBlockSig.clear();
    }

    CBlockHeader GetBlockHeader() const
    {
        CBlockHeader block;
        block.nVersion       = nVersion;
        block.hashPrevBlock  = hashPrevBlock;
        block.hashMerkleRoot = hashMerkleRoot;
        block.nTime          = nTime;
        block.nBits          = nBits;
        block.nNonce         = nNonce;
        block.nAccumulatorCheckpoint = nAccumulatorCheckpoint;
#ifdef  POW_IN_POS_PHASE
        block.nBits2        = nBits2;
#endif
        return block;
    }

    VM_STATE_ROOT GetVMState(uint256 &hashStateRoot, uint256 &hashUTXORoot) const;

    // ppcoin: two types of block: proof-of-work or proof-of-stake
    bool IsProofOfStake() const
    {
        return (vtx.size() > 1 && vtx[1].IsCoinStake());
    }

    bool IsProofOfWork() const
    {
        return !IsProofOfStake();
    }

    bool IsContractEnabled() const
    {
        return (nVersion > ZEROCOIN_VERSION);
    }

    bool SignBlock(const CKeyStore& keystore);
    bool CheckBlockSignature() const;

    std::pair<COutPoint, unsigned int> GetProofOfStake() const
    {
        return IsProofOfStake()? std::make_pair(vtx[1].vin[0].prevout, nTime) : std::make_pair(COutPoint(), (unsigned int)0);
    }

    // Build the in-memory merkle tree for this block and return the merkle root.
    // If non-NULL, *mutated is set to whether mutation was detected in the merkle
    // tree (a duplication of transactions in the block leading to an identical
    // merkle root).
    uint256 BuildMerkleTree(bool* mutated = NULL) const;

    std::vector<uint256> GetMerkleBranch(int nIndex) const;
    static uint256 CheckMerkleBranch(uint256 hash, const std::vector<uint256>& vMerkleBranch, int nIndex);
    std::string ToString() const;
    void print() const;
};


/** Describes a place in the block chain to another node such that if the
 * other node doesn't have the same branch, it can find a recent common trunk.
 * The further back it is, the further before the fork it may be.
 */
struct CBlockLocator
{
    std::vector<uint256> vHave;

    CBlockLocator() {}

    CBlockLocator(const std::vector<uint256>& vHaveIn)
    {
        vHave = vHaveIn;
    }

    ADD_SERIALIZE_METHODS;

    template <typename Stream, typename Operation>
    inline void SerializationOp(Stream& s, Operation ser_action, int nType, int nVersion) {
        if (!(nType & SER_GETHASH))
            READWRITE(nVersion);
        READWRITE(vHave);
    }

    void SetNull()
    {
        vHave.clear();
    }

    bool IsNull()
    {
        return vHave.empty();
    }
};

#endif // BITCOIN_PRIMITIVES_BLOCK_H
