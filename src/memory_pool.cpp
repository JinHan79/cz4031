#include "memory_pool.h"
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>

using namespace std;

MemoryPool::MemoryPool(unsigned int poolSize, unsigned int blockSize) {
    this->poolSize = poolSize;
    this->blockSize = blockSize;
    uchar* poolPtr = nullptr; 
    this->poolPtr = new unsigned char[poolSize]; 
    this->blockPtr = nullptr;
    this->blocksUsed = 0;
    this->recordsUsed = 0;
    this->blocksAllocated = 0;
    this->blocksAvailable = poolSize / blockSize;
    this->curBlock = 0;
}

bool MemoryPool::allocBlock() {

    if (numAvailBlocks > 0) {
        blockPtr = poolPtr + (blocksAllocated * blockSize); 
        blocksUsed += blockSize;
        blocksAvailable -= 1;
        blocksAllocated += 1;
        curBlock = 0;

        return true;
    }

    else {
        cout << "Memory is full!";
        return false;
    }
}

tuple<void * , unsigned int> MemoryPool::writeRecord(unsigned int recordSize) {

    if(blockSize < (curBlock + recordSize) or blocksAllocated == 0){
        if (allocBlk() == false)
            throw "Unable to reserve space. No free space in blocks or no blocks can be allocated";
    }

    if (blockSize < recordSize) {
        throw "Unable to reserve space. Record size is greater than the block size";
    }

    tuple<void * , unsigned int> recordAddress(blockPtr, curBlock);

    recordsUsed += recordSize;
    curBlock += recordSize;

    return recordAddress;
}


bool MemoryPool::deleteRecord(unsigned char *blockAddress, unsigned int offset, unsigned int recordSize) {
    try {
        recordsUsed -= recordSize;
        fill(blockAddress + offset, blockAddress + offset + recordSize, '\0');

        unsigned char cmpBlk [recordSize];
        fill(cmpBlk, cmpBlk+recordSize, '\0');

        if(equal(cmpBlk, cmpBlk+recordSize, blockAddress)){
            blocksUsed -= blockSize;
        }

        return true;
    }

    catch(exception &e) {
        cout << "Exception" << e.what() << "\n";
        cout << "Failed to delete record or block" << "\n";
        return false;
    }
}


MemoryPool::~MemoryPool() {
    delete poolPtr;
    poolPtr = nullptr;
}
