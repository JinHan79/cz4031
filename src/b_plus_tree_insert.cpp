/*
Created by Jin Han
For CZ4031 B+ Tree implementation and experiments project

This file implements insertion on a b+ tree
*/

// include all the files that are needed
#include "bplustree.h"
#include "types.h"

// include all the libraries needed
#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

// To insert a movie into the B+ Tree index. Key: Movies's numVotes, Value: {blockAddress, offset}
void BPlusTree::insert(Address address, int key)
{
    maxKeyCount = getMaxKeyCount();

    if (rootAddress == nullptr)
    {
        Node* LLNode = new Node(maxKeyCount);
        LLNode->keys[0] = key;
        LLNode->isLeaf = false;
        LLNode->curKeyCount = 1;
        LLNode->pointers[0] = address;

        Address LLNodeAddress = index->saveToDisk((void*)LLNode, nodeSize);

        root = new Node(maxKeyCount);
        root->keys[0] = key;
        root->isLeaf = true;
        root->curKeyCount = 1;
        root->pointers[0] = LLNodeAddress;

        rootAddress = index->saveToDisk(root, nodeSize).blockAddress;
    }
    else
    {
        Node* cursor = root;
        Node* parent;
        void* parentDiskAddress = rootAddress;
        void* cursorDiskAddress = rootAddress;

        while (cursor->isLeaf == false)
        {
            parent = cursor;
            parentDiskAddress = cursorDiskAddress;

            for (int i = 0; i < cursor->curKeyCount; i++)
            {
                if (key < cursor->keys[i])
                {
                    Node* mainMemoryNode = (Node*)index->loadFromDisk(cursor->pointers[i], nodeSize);
                    cursorDiskAddress = cursor->pointers[i].blockAddress;
                    cursor = mainMemoryNode;
                    break;
                }
                if (i == cursor->curKeyCount - 1)
                {
                    Node* mainMemoryNode = (Node*)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);
                    cursorDiskAddress = cursor->pointers[i + 1].blockAddress;
                    cursor = (Node*)mainMemoryNode;
                    break;
                }
            }
        }

        // If the leaf node has an expty space, insert a record
        if (cursor->curKeyCount < maxKeyCount)
        {
            int i = 0;
            while (key > cursor->keys[i] && i < cursor->curKeyCount)
            {
                i++;
            }

            if (cursor->keys[i] == key)
            {
                cursor->pointers[i] = insertLL(cursor->pointers[i], address, key);
            }
            else
            {
                Address next = cursor->pointers[cursor->curKeyCount];

                for (int j = cursor->curKeyCount; j > i; j--)
                {
                    cursor->keys[j] = cursor->keys[j - 1];
                    cursor->pointers[j] = cursor->pointers[j - 1];
                }
                cursor->keys[i] = key;

                Node* LLNode = new Node(maxKeyCount);
                LLNode->keys[0] = key;
                LLNode->isLeaf = false;
                LLNode->curKeyCount = 1;
                LLNode->pointers[0] = address;
                Address LLNodeAddress = index->saveToDisk((void*)LLNode, nodeSize);
                cursor->pointers[i] = LLNodeAddress;
                cursor->curKeyCount++;
                cursor->pointers[cursor->curKeyCount] = next;
                Address cursorOriginalAddress{ cursorDiskAddress, 0 };
                index->saveToDisk(cursor, nodeSize, cursorOriginalAddress);
            }
        }

        // If the node is full, split it into two nodes
        else
        {
            Node* newLeaf = new Node(maxKeyCount);
            float tempKeyList[maxKeyCount + 1];

            Address tempPointerList[maxKeyCount + 1];
            Address next = cursor->pointers[cursor->curKeyCount];

            int i = 0;
            for (i = 0; i < maxKeyCount; i++)
            {
                tempKeyList[i] = cursor->keys[i];
                tempPointerList[i] = cursor->pointers[i];
            }

            i = 0;
            while (key > tempKeyList[i] && i < maxKeyCount)
            {
                i++;
            }

            if (i < cursor->curKeyCount) {
                if (cursor->keys[i] == key)
                {
                    cursor->pointers[i] = insertLL(cursor->pointers[i], address, key);
                    return;
                }
            }

            for (int j = maxKeyCount; j > i; j--)
            {
                tempKeyList[j] = tempKeyList[j - 1];
                tempPointerList[j] = tempPointerList[j - 1];
            }

            tempKeyList[i] = key;

            Node* LLNode = new Node(maxKeyCount);
            LLNode->keys[0] = key;
            LLNode->isLeaf = false;
            LLNode->curKeyCount = 1;
            LLNode->pointers[0] = address;

            Address LLNodeAddress = index->saveToDisk((void*)LLNode, nodeSize);
            tempPointerList[i] = LLNodeAddress;

            newLeaf->isLeaf = true;
            cursor->curKeyCount = (maxKeyCount + 1) / 2;
            newLeaf->curKeyCount = (maxKeyCount + 1) - ((maxKeyCount + 1) / 2);
            newLeaf->pointers[newLeaf->curKeyCount] = next;

            for (i = 0; i < cursor->curKeyCount; i++)
            {
                cursor->keys[i] = tempKeyList[i];
                cursor->pointers[i] = tempPointerList[i];
            }

            for (int j = 0; j < newLeaf->curKeyCount; i++, j++)
            {
                newLeaf->keys[j] = tempKeyList[i];
                newLeaf->pointers[j] = tempPointerList[i];
            }

            Address newLeafAddress = index->saveToDisk(newLeaf, nodeSize);
            cursor->pointers[cursor->curKeyCount] = newLeafAddress;

            for (int i = cursor->curKeyCount; i < maxKeyCount; i++) {
                cursor->keys[i] = float();
            }
            for (int i = cursor->curKeyCount + 1; i < maxKeyCount + 1; i++) {
                Address nullAddress{ nullptr, 0 };
                cursor->pointers[i] = nullAddress;
            }

            Address cursorOriginalAddress{ cursorDiskAddress, 0 };
            index->saveToDisk(cursor, nodeSize, cursorOriginalAddress);

            if (cursor == root)
            {
                Node* newRoot = new Node(maxKeyCount);
                newRoot->keys[0] = newLeaf->keys[0];
                Address cursorDisk{ cursorDiskAddress, 0 };

                newRoot->pointers[0] = cursorDisk;
                newRoot->pointers[1] = newLeafAddress;
                newRoot->isLeaf = false;
                newRoot->curKeyCount = 1;

                Address newRootAddress = index->saveToDisk(newRoot, nodeSize);
                rootAddress = newRootAddress.blockAddress;
                root = newRoot;
            }

            else
            {
                insertInternal(newLeaf->keys[0], (Node*)parentDiskAddress, (Node*)newLeafAddress.blockAddress);
            }
        }
    }
}

void BPlusTree::insertInternal(int key, Node* cursorDiskAddress, Node* childDiskAddress)
{
    maxKeyCount = getMaxKeyCount();
    Address cursorAddress{ cursorDiskAddress, 0 };
    Node* cursor = (Node*)index->loadFromDisk(cursorAddress, nodeSize);

    if (cursorDiskAddress == rootAddress)
    {
        root = cursor;
    }

    Address childAddress{ childDiskAddress, 0 };
    Node* child = (Node*)index->loadFromDisk(childAddress, nodeSize);

    if (cursor->curKeyCount < maxKeyCount)
    {
        int i = 0;
        while (key > cursor->keys[i] && i < cursor->curKeyCount)
        {
            i++;
        }

        for (int j = cursor->curKeyCount; j > i; j--)
        {
            cursor->keys[j] = cursor->keys[j - 1];
        }

        for (int j = cursor->curKeyCount + 1; j > i + 1; j--)
        {
            cursor->pointers[j] = cursor->pointers[j - 1];
        }

        cursor->keys[i] = key;
        cursor->curKeyCount++;

        Address childAddress{ childDiskAddress, 0 };
        cursor->pointers[i + 1] = childAddress;
        Address cursorAddress{ cursorDiskAddress, 0 };
        index->saveToDisk(cursor, nodeSize, cursorAddress);
    }

    else
    {
        Node* newInternal = new Node(maxKeyCount);

        float tempKeyList[maxKeyCount + 1];
        Address tempPointerList[maxKeyCount + 2];

        for (int i = 0; i < maxKeyCount; i++)
        {
            tempKeyList[i] = cursor->keys[i];
        }

        for (int i = 0; i < maxKeyCount + 1; i++)
        {
            tempPointerList[i] = cursor->pointers[i];
        }

        int i = 0;
        while (key > tempKeyList[i] && i < maxKeyCount)
        {
            i++;
        }

        int j;
        for (int j = maxKeyCount; j > i; j--)
        {
            tempKeyList[j] = tempKeyList[j - 1];
        }

        tempKeyList[i] = key;

        for (int j = maxKeyCount + 1; j > i + 1; j--)
        {
            tempPointerList[j] = tempPointerList[j - 1];
        }

        Address childAddress = { childDiskAddress, 0 };
        tempPointerList[i + 1] = childAddress;
        newInternal->isLeaf = false;
        cursor->curKeyCount = (maxKeyCount + 1) / 2;
        newInternal->curKeyCount = maxKeyCount - (maxKeyCount + 1) / 2;

        for (int i = 0; i < cursor->curKeyCount; i++)
        {
            cursor->keys[i] = tempKeyList[i];
        }

        for (i = 0, j = cursor->curKeyCount + 1; i < newInternal->curKeyCount; i++, j++)
        {
            newInternal->keys[i] = tempKeyList[j];
        }

        for (i = 0, j = cursor->curKeyCount + 1; i < newInternal->curKeyCount + 1; i++, j++)
        {
            newInternal->pointers[i] = tempPointerList[j];
        }

        for (int i = cursor->curKeyCount; i < maxKeyCount; i++)
        {
            cursor->keys[i] = float();
        }

        for (int i = cursor->curKeyCount + 1; i < maxKeyCount + 1; i++)
        {
            Address nullAddress{ nullptr, 0 };
            cursor->pointers[i] = nullAddress;
        }

        cursor->pointers[cursor->curKeyCount] = childAddress;
        Address cursorAddress{ cursorDiskAddress, 0 };
        index->saveToDisk(cursor, nodeSize, cursorAddress);
        Address newInternalDiskAddress = index->saveToDisk(newInternal, nodeSize);

        if (cursor == root)
        {
            Node* newRoot = new Node(nodeSize);
            newRoot->keys[0] = cursor->keys[cursor->curKeyCount];
            Address cursorAddress = { cursorDiskAddress, 0 };
            newRoot->pointers[0] = cursorAddress;
            newRoot->pointers[1] = newInternalDiskAddress;
            newRoot->isLeaf = false;
            newRoot->curKeyCount = 1;
            root = newRoot;
            Address newRootAddress = index->saveToDisk(root, nodeSize);
            rootAddress = newRootAddress.blockAddress;
        }
        else
        {
            Node* parentDiskAddress = findParent((Node*)rootAddress, cursorDiskAddress, cursor->keys[0]);
            insertInternal(tempKeyList[cursor->curKeyCount], parentDiskAddress, (Node*)newInternalDiskAddress.blockAddress);
        }
    }
}

Address BPlusTree::insertLL(Address LLHead, Address address, int key)
{
    Node* head = (Node*)index->loadFromDisk(LLHead, nodeSize);

    if (head->curKeyCount < maxKeyCount)
    {
        for (int i = head->curKeyCount; i > 0; i--)
        {
            head->keys[i] = head->keys[i - 1];
        }

        for (int i = head->curKeyCount + 1; i > 0; i--)

        {
            head->pointers[i] = head->pointers[i - 1];
        }

        head->keys[0] = key;
        head->pointers[0] = address;
        head->curKeyCount++;
        LLHead = index->saveToDisk((void*)head, nodeSize, LLHead);

        return LLHead;
    }

    else
    {
        Node* LLNode = new Node(maxKeyCount);
        LLNode->isLeaf = false;
        LLNode->keys[0] = key;
        LLNode->curKeyCount = 1;
        LLNode->pointers[0] = address;
        LLNode->pointers[1] = LLHead;
        Address LLNodeAddress = index->saveToDisk((void*)LLNode, nodeSize);
        return LLNodeAddress;
    }
}