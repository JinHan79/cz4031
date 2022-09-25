/*
Created by Bob Lin
For CZ4031 B+ Tree implementation and experiments project

This file is the header file for bplustree.cpp
*/

#ifndef BPLUSTREE_H
#define BPLUSTREE_H

// Include all the files that are needed
#include "types.h"
#include "memory_pool.h"

// Include all the libraries needed
#include <cstddef>
#include <array>

// Represents a typical node in a b+ tree
class Node
{
    // Variables are all private
private:
    Address* pointers;      // Pointer to an array of pointers that exist in this node - a struct {void *blockAddress, short int offset} containing other nodes in disk.
    int* keys;            // Pointer to an array of keys that exist in this node
    bool isLeaf;            // Boolean whether this is a leaf node or not
    int curKeyCount;       // Current number of keys in this node.
    friend class BPlusTree; // Only BPlusTree can access the private variables in Node class

    // Functions are all public (to be used by others)
public:
    // Constructor
    Node(int maxKeyCount);

};

// The actual b+ tree structure
class BPlusTree
{
private:
    // Variables
    MemoryPool* index;    // Pointer to a memory pool in disk for index
    MemoryPool* disk;     // Pointer to a memory pool for data blocks
    Node* root;           // Pointer to the root node
    void* rootAddress;    // Pointer to root's disk address 
    int maxKeyCount;      // Max num of keys in one node
    int levelsCount;      // Levels or height of this b+ tree
    int nodesCount;       // Count of num of nodes in b+ tree
    std::size_t nodeSize; // Size of node

    // Functions (Private)
    void insertInternal(int key, Node* cursorDiskAddress, Node* childDiskAddress);
    void removeInternal(int key, Node* cursorDiskAddress, Node* childDiskAddress);
    Node* findParent(Node*, Node*, int lowerBoundKey);

public:
    // Functions (Public)

    // Constructor - input blockSize is used in constructor to find out how many keys and pointers can fit in a node.
    BPlusTree(std::size_t blockSize, MemoryPool* disk, MemoryPool* index);

    // Search - TODO by Kai Feng
    void search(int lowerBoundKey, int upperBoundKey);

    // Insert and Delete - TODO by Jin Han
    void insert(Address address, int key);
    // Insert to Linked List (for overflow chain)
    Address insertLL(Address LLHead, Address address, int key);
    // Delete 
    int remove(int key);
    // Delete Linked List
    void removeLL(Address LLHeadAddress);

    // Display functions - TODO by Gareth
    void display(Node*, int level);
    void displayNode(Node* node);
    void displayBlock(void* block);
    void displayLL(Address LLHeadAddress);


    // Functions to get and set

    // Returns pointer to the root of b+ tree
    Node* getRoot()
    {
        return root;
    };

    // Returns levelsCount of b+ tree
    int getLevels();

    // Returns nodesCount of b+ tree
    int getNodesCount()
    {
        return nodesCount;
    }

    // Returns maxNumOfKeys of b+ tree
    int getMaxKeyCount()
    {
        return maxKeyCount;
    }

};

#endif