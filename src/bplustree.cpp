/*
Created by Bob Lin
For CZ4031 B+ Tree implementation and experiments project

This file implements the b+ tree (initialisation of b+ tree and some extra functions)
*/

// include all the files that are needed
#include "types.h"
#include "memory_pool.h"
#include "bplustree.h"

// include all the libraries needed
#include <iostream>
#include <unordered_map>
#include <cstring>
#include <array>
#include <tuple>
#include <vector>

using namespace std;
bool myNullPtr = false;

// To initialize Node type
Node::Node(int maxKeyCount)
{
    // Initialize keys
    keys = new float[maxKeyCount];
    // Initialize pointers
    pointers = new Address[maxKeyCount + 1];

    for (int i = 0; i < maxKeyCount + 1; i++)
    {
        Address nullAddress{ (void*)myNullPtr, 0 };
        pointers[i] = nullAddress;
    }
    curKeyCount = 0;
}

// To define the b+ tree structure + fill it with keys and pointers according to how much space available
BPlusTree::BPlusTree(std::size_t blockSize, MemoryPool* disk, MemoryPool* index)
{
    // sizeAvailable is the size remaining to accomodate any keys/pointers after the space used by isLeaf flag and count of keys (curNumOfKeys)
    size_t sizeAvailable = blockSize - (sizeof(int) + sizeof(bool));

    // No. of pointers = No. of keys + 1, so we first add in 1 pointer before we add key,pointer pairs
    size_t sizeSoFar = sizeof(Address);
    maxKeyCount = 0;

    // Iterate to add key,pointer pairs until no more space available
    while (sizeSoFar + sizeof(Address) + sizeof(float) <= sizeAvailable)
    {
        sizeSoFar += (sizeof(Address) + sizeof(float));
        maxKeyCount += 1;
    }

    if (maxKeyCount == 0)
    {
        throw std::overflow_error("Error: Keys and pointers too large to fit into a node!");
    }

    // Initialize root to NULL
    rootAddress = nullptr;
    root = nullptr;

    // Set node size to be equal to block size.
    nodeSize = blockSize;

    // Initialize initial variables
    levelsCount = 0;
    nodesCount = 0;

    // Initialize disk space for index and set reference to disk.

    this->disk = disk;
    this->index = index;
}


// Some extra functions related to b+ tree

// Find the parent of a node.
Node* BPlusTree::findParent(Node* cursorDiskAddress, Node* childDiskAddress, float lowerBoundKey)
{
    // Load in cursor into main memory, starting from root.
    Address cursorAddress{ cursorDiskAddress, 0 };
    Node* cursor = (Node*)index->loadFromDisk(cursorAddress, nodeSize);

    // If the root cursor passed in is a leaf node, there is no children, therefore no parent.
    if (cursor->isLeaf)
    {
        return nullptr;
    }

    // Maintain parentDiskAddress
    Node* parentDiskAddress = cursorDiskAddress;

    // While not leaf, keep following the nodes to correct key.
    while (cursor->isLeaf == false)
    {
        // Check through all pointers of the node to find match.
        for (int i = 0; i < cursor->curKeyCount + 1; i++)
        {
            if (cursor->pointers[i].blockAddress == childDiskAddress)
            {
                return parentDiskAddress;
            }
        }

        for (int i = 0; i < cursor->curKeyCount; i++)
        {
            // If key is lesser than current key, go to the left pointer's node.
            if (lowerBoundKey < cursor->keys[i])
            {
                // Load node in from disk to main memory.
                Node* mainMemoryNode = (Node*)index->loadFromDisk(cursor->pointers[i], nodeSize);

                // Update parent address.
                parentDiskAddress = (Node*)cursor->pointers[i].blockAddress;

                // Move to new node in main memory.
                cursor = (Node*)mainMemoryNode;
                break;
            }

            // Else if key larger than all keys in the node, go to last pointer's node (rightmost).
            if (i == cursor->curKeyCount - 1)
            {
                // Load node in from disk to main memory.
                Node* mainMemoryNode = (Node*)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);

                // Update parent address.
                parentDiskAddress = (Node*)cursor->pointers[i + 1].blockAddress;

                // Move to new node in main memory.
                cursor = (Node*)mainMemoryNode;
                break;
            }
        }
    }

    // If we reach here, means cannot find already.
    return nullptr;
}


int BPlusTree::getLevels() {

    if (rootAddress == nullptr) {
        return 0;
    }

    // Load in the root node from disk
    Address rootDiskAddress{ rootAddress, 0 };
    root = (Node*)index->loadFromDisk(rootDiskAddress, nodeSize);
    Node* cursor = root;

    levelsCount = 1;

    while (!cursor->isLeaf) {
        cursor = (Node*)index->loadFromDisk(cursor->pointers[0], nodeSize);
        levelsCount++;
    }

    // Account for linked list (count as one level)
    levelsCount++;

    return levelsCount;
}
