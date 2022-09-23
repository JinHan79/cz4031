#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H
#include <cstring>
#include <iostream>
#include <vector>
#include <tuple>
#include <algorithm>

const int MAX = 5;

using namespace std;

struct Record {
    char tconst[10];
    float averageRating;
    unsigned int numVotes;
};

class MemmoryPool {
private:
    unsigned char *poolPtr;
    unsigned char *blockPtr;

    unsigned int poolSize;
    unsigned int blockSize;
    unsigned int blocksUsed;
    unsigned int recordsUsed;
    unsigned int curBlock;

    int blocksAllocated;
    int blocksAvailable;

public:
    MemoryPool(unsigned int poolSize, unsigned int blockSize);

    ~MemoryPool();

    bool allocBlock();

    tuple<void *, unsigned int> writeRecord(unsigned int recordSize);

    bool deleteRecord(unsigned char *blockAddress, unsigned int offset, unsigned int recordSize);

    unsigned int getBlockSize(){
        return blockSize;
    }

    unsigned int getPoolSize() {
        return poolSize;
    }

    int getBlocksUsed() {
        return blocksUsed;
    }

    int getRecordsUsed() {
        return recordsUsed;
    }

    int getCurBlock(){
        return curBlock;
    }

    int getBlocksAllocated() {
        return blocksAllocated;
    }

    int getBlocksAvailable() {
        return blocksAvailable;
    }
};

#endif
