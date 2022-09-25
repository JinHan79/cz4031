/*
Created by Jin Han
For CZ4031 B+ Tree implementation and experiments project

This file implements deletion on a b+ tree
*/

// include all the files that are needed
#include "bplustree.h"
#include "types.h"

// include all the libraries needed
#include <vector>
#include <cstring>
#include <iostream>

using namespace std;

// To delete a movie from the B+ Tree index. Key: Movies's numVotes
int BPlusTree::remove(int key)
{
    int numNodes = index->getAllocated();

    // If no nodes in B+ Tree
    if (rootAddress == nullptr)
    {
        throw std::logic_error("There are no movies stored");
    }
    else
    {
        Address rootDiskAddress{ rootAddress, 0 };
        root = (Node*)index->loadFromDisk(rootDiskAddress, nodeSize);
        Node* cursor = root;
        Node* parent;
        void* parentDiskAddress = rootAddress;
        void* cursorDiskAddress = rootAddress;
        int leftSibling, rightSibling;

        while (cursor->isLeaf == false)
        {
            parent = cursor;
            parentDiskAddress = cursorDiskAddress;

            for (int i = 0; i < cursor->curKeyCount; i++)
            {
                leftSibling = i - 1;
                rightSibling = i + 1;

                if (key < cursor->keys[i])
                {
                    Node* mainMemoryNode = (Node*)index->loadFromDisk(cursor->pointers[i], nodeSize);
                    cursorDiskAddress = cursor->pointers[i].blockAddress;
                    cursor = (Node*)mainMemoryNode;
                    break;
                }

                if (i == cursor->curKeyCount - 1)
                {
                    leftSibling = i;
                    rightSibling = i + 2;
                    Node* mainMemoryNode = (Node*)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);
                    cursorDiskAddress = cursor->pointers[i + 1].blockAddress;
                    cursor = (Node*)mainMemoryNode;
                    break;
                }
            }
        }

        bool found = false;
        int position;

        for (position = 0; position < cursor->curKeyCount; position++)
        {
            if (cursor->keys[position] == key)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::cout << "Target key is not found: " << key << endl;
            int numNodesDeleted = numNodes - index->getAllocated();
            numNodes = index->getAllocated();
            return numNodesDeleted;
        }

        removeLL(cursor->pointers[position]);

        for (int i = position; i < cursor->curKeyCount; i++)
        {
            cursor->keys[i] = cursor->keys[i + 1];
            cursor->pointers[i] = cursor->pointers[i + 1];
        }

        cursor->curKeyCount--;
        cursor->pointers[cursor->curKeyCount] = cursor->pointers[cursor->curKeyCount + 1];

        for (int i = cursor->curKeyCount + 1; i < maxKeyCount + 1; i++)
        {
            Address nullAddress{ nullptr, 0 };
            cursor->pointers[i] = nullAddress;
        }

        if (cursor == root)
        {
            if (cursor->curKeyCount == 0)
            {
                std::cout << "Succesfully deleted the entire index" << endl;

                Address rootDiskAddress{ rootAddress, 0 };
                index->deallocate(rootDiskAddress, nodeSize);
                root = nullptr;
                rootAddress = nullptr;
            }

            std::cout << "Successfully deleted key: " << key << endl;
            int numNodesDeleted = numNodes - index->getAllocated();
            numNodes = index->getAllocated();
            Address cursorAddress = { cursorDiskAddress, 0 };
            index->saveToDisk(cursor, nodeSize, cursorAddress);
            return numNodesDeleted;
        }

        if (cursor->curKeyCount >= (maxKeyCount + 1) / 2)
        {
            std::cout << "Successfully deleted key: " << key << endl;
            int numNodesDeleted = numNodes - index->getAllocated();
            numNodes = index->getAllocated();
            Address cursorAddress = { cursorDiskAddress, 0 };
            index->saveToDisk(cursor, nodeSize, cursorAddress);
            return numNodesDeleted;
        }

        if (leftSibling >= 0)
        {
            Node* leftNode = (Node*)index->loadFromDisk(parent->pointers[leftSibling], nodeSize);

            if (leftNode->curKeyCount >= (maxKeyCount + 1) / 2 + 1)
            {
                cursor->pointers[cursor->curKeyCount + 1] = cursor->pointers[cursor->curKeyCount];

                for (int i = cursor->curKeyCount; i > 0; i--)
                {
                    cursor->keys[i] = cursor->keys[i - 1];
                    cursor->pointers[i] = cursor->pointers[i - 1];
                }

                cursor->keys[0] = leftNode->keys[leftNode->curKeyCount - 1];
                cursor->pointers[0] = leftNode->pointers[leftNode->curKeyCount - 1];
                cursor->curKeyCount++;
                leftNode->curKeyCount--;
                leftNode->pointers[cursor->curKeyCount] = leftNode->pointers[cursor->curKeyCount + 1];
                parent->keys[leftSibling] = cursor->keys[0];
                Address parentAddress{ parentDiskAddress, 0 };
                index->saveToDisk(parent, nodeSize, parentAddress);
                index->saveToDisk(leftNode, nodeSize, parent->pointers[leftSibling]);
                Address cursorAddress = { cursorDiskAddress, 0 };
                index->saveToDisk(cursor, nodeSize, cursorAddress);
                int numNodesDeleted = numNodes - index->getAllocated();
                numNodes = index->getAllocated();
                return numNodesDeleted;
            }
        }

        if (rightSibling <= parent->curKeyCount)
        {
            Node* rightNode = (Node*)index->loadFromDisk(parent->pointers[rightSibling], nodeSize);

            if (rightNode->curKeyCount >= (maxKeyCount + 1) / 2 + 1)
            {
                cursor->pointers[cursor->curKeyCount + 1] = cursor->pointers[cursor->curKeyCount];
                cursor->keys[cursor->curKeyCount] = rightNode->keys[0];
                cursor->pointers[cursor->curKeyCount] = rightNode->pointers[0];
                cursor->curKeyCount++;
                rightNode->curKeyCount--;

                for (int i = 0; i < rightNode->curKeyCount; i++)
                {
                    rightNode->keys[i] = rightNode->keys[i + 1];
                    rightNode->pointers[i] = rightNode->pointers[i + 1];
                }

                rightNode->pointers[cursor->curKeyCount] = rightNode->pointers[cursor->curKeyCount + 1];
                parent->keys[rightSibling - 1] = rightNode->keys[0];
                Address parentAddress{ parentDiskAddress, 0 };
                index->saveToDisk(parent, nodeSize, parentAddress);
                index->saveToDisk(rightNode, nodeSize, parent->pointers[rightSibling]);
                Address cursorAddress = { cursorDiskAddress, 0 };
                index->saveToDisk(cursor, nodeSize, cursorAddress);
                int numNodesDeleted = numNodes - index->getAllocated();
                numNodes = index->getAllocated();
                return numNodesDeleted;
            }
        }

        if (leftSibling >= 0)
        {
            Node* leftNode = (Node*)index->loadFromDisk(parent->pointers[leftSibling], nodeSize);

            for (int i = leftNode->curKeyCount, j = 0; j < cursor->curKeyCount; i++, j++)
            {
                leftNode->keys[i] = cursor->keys[j];
                leftNode->pointers[i] = cursor->pointers[j];
            }

            leftNode->curKeyCount += cursor->curKeyCount;
            leftNode->pointers[leftNode->curKeyCount] = cursor->pointers[cursor->curKeyCount];
            index->saveToDisk(leftNode, nodeSize, parent->pointers[leftSibling]);
            removeInternal(parent->keys[leftSibling], (Node*)parentDiskAddress, (Node*)cursorDiskAddress);
            Address cursorAddress{ cursorDiskAddress, 0 };
            index->deallocate(cursorAddress, nodeSize);
        }
        else if (rightSibling <= parent->curKeyCount)
        {
            Node* rightNode = (Node*)index->loadFromDisk(parent->pointers[rightSibling], nodeSize);

            for (int i = cursor->curKeyCount, j = 0; j < rightNode->curKeyCount; i++, j++)
            {
                cursor->keys[i] = rightNode->keys[j];
                cursor->pointers[i] = rightNode->pointers[j];
            }

            cursor->curKeyCount += rightNode->curKeyCount;
            cursor->pointers[cursor->curKeyCount] = rightNode->pointers[rightNode->curKeyCount];
            Address cursorAddress{ cursorDiskAddress, 0 };
            index->saveToDisk(cursor, nodeSize, cursorAddress);
            void* rightNodeAddress = parent->pointers[rightSibling].blockAddress;
            removeInternal(parent->keys[rightSibling - 1], (Node*)parentDiskAddress, (Node*)rightNodeAddress);
            Address rightNodeDiskAddress{ rightNodeAddress, 0 };
            index->deallocate(rightNodeDiskAddress, nodeSize);
        }
    }

    int numNodesDeleted = numNodes - index->getAllocated();
    numNodes = index->getAllocated();
    return numNodesDeleted;
}

void BPlusTree::removeInternal(int key, Node* cursorDiskAddress, Node* childDiskAddress)
{
    Address cursorAddress{ cursorDiskAddress, 0 };
    Node* cursor = (Node*)index->loadFromDisk(cursorAddress, nodeSize);

    if (cursorDiskAddress == rootAddress)
    {
        root = cursor;
    }

    Address childAddress{ childDiskAddress, 0 };

    if (cursor == root)
    {
        if (cursor->curKeyCount == 1)
        {
            if (cursor->pointers[1].blockAddress == childDiskAddress)
            {
                index->deallocate(childAddress, nodeSize);
                root = (Node*)index->loadFromDisk(cursor->pointers[0], nodeSize);
                rootAddress = (Node*)cursor->pointers[0].blockAddress;
                index->deallocate(cursorAddress, nodeSize);
                std::cout << "Root node changed." << endl;
                return;
            }
            else if (cursor->pointers[0].blockAddress == childDiskAddress)
            {
                index->deallocate(childAddress, nodeSize);
                root = (Node*)index->loadFromDisk(cursor->pointers[1], nodeSize);
                rootAddress = (Node*)cursor->pointers[1].blockAddress;
                index->deallocate(cursorAddress, nodeSize);
                std::cout << "Root node changed." << endl;
                return;
            }
        }
    }

    int position;
    for (position = 0; position < cursor->curKeyCount; position++)
    {
        if (cursor->keys[position] == key)
        {
            break;
        }
    }

    for (int i = position; i < cursor->curKeyCount; i++)
    {
        cursor->keys[i] = cursor->keys[i + 1];
    }

    for (position = 0; position < cursor->curKeyCount + 1; position++)
    {
        if (cursor->pointers[position].blockAddress == childDiskAddress)
        {
            break;
        }
    }

    for (int i = position; i < cursor->curKeyCount + 1; i++)
    {
        cursor->pointers[i] = cursor->pointers[i + 1];
    }

    cursor->curKeyCount--;

    if (cursor->curKeyCount >= (maxKeyCount + 1) / 2 - 1)
    {
        return;
    }

    if (cursorDiskAddress == rootAddress)
    {
        return;
    }

    Node* parentDiskAddress = findParent((Node*)rootAddress, cursorDiskAddress, cursor->keys[0]);
    int leftSibling, rightSibling;
    Address parentAddress{ parentDiskAddress, 0 };
    Node* parent = (Node*)index->loadFromDisk(parentAddress, nodeSize);

    for (position = 0; position < parent->curKeyCount + 1; position++)
    {
        if (parent->pointers[position].blockAddress == cursorDiskAddress)
        {
            leftSibling = position - 1;
            rightSibling = position + 1;
            break;
        }
    }

    if (leftSibling >= 0)
    {
        Node* leftNode = (Node*)index->loadFromDisk(parent->pointers[leftSibling], nodeSize);

        if (leftNode->curKeyCount >= (maxKeyCount + 1) / 2)
        {
            for (int i = cursor->curKeyCount; i > 0; i--)
            {
                cursor->keys[i] = cursor->keys[i - 1];
            }

            cursor->keys[0] = parent->keys[leftSibling];
            parent->keys[leftSibling] = leftNode->keys[leftNode->curKeyCount - 1];

            for (int i = cursor->curKeyCount + 1; i > 0; i--)
            {
                cursor->pointers[i] = cursor->pointers[i - 1];
            }

            cursor->pointers[0] = leftNode->pointers[leftNode->curKeyCount];
            cursor->curKeyCount++;
            leftNode->curKeyCount--;
            leftNode->pointers[cursor->curKeyCount] = leftNode->pointers[cursor->curKeyCount + 1];

            Address parentAddress{ parentDiskAddress, 0 };
            index->saveToDisk(parent, nodeSize, parentAddress);
            index->saveToDisk(leftNode, nodeSize, parent->pointers[leftSibling]);
            Address cursorAddress = { cursorDiskAddress, 0 };
            index->saveToDisk(cursor, nodeSize, cursorAddress);
            return;
        }
    }

    if (rightSibling <= parent->curKeyCount)
    {
        Node* rightNode = (Node*)index->loadFromDisk(parent->pointers[rightSibling], nodeSize);

        if (rightNode->curKeyCount >= (maxKeyCount + 1) / 2)
        {
            cursor->keys[cursor->curKeyCount] = parent->keys[position];
            parent->keys[position] = rightNode->keys[0];

            for (int i = 0; i < rightNode->curKeyCount - 1; i++)
            {
                rightNode->keys[i] = rightNode->keys[i + 1];
            }

            cursor->pointers[cursor->curKeyCount + 1] = rightNode->pointers[0];

            for (int i = 0; i < rightNode->curKeyCount; ++i)
            {
                rightNode->pointers[i] = rightNode->pointers[i + 1];
            }

            cursor->curKeyCount++;
            rightNode->curKeyCount--;

            Address parentAddress{ parentDiskAddress, 0 };
            index->saveToDisk(parent, nodeSize, parentAddress);
            index->saveToDisk(rightNode, nodeSize, parent->pointers[rightSibling]);
            Address cursorAddress = { cursorDiskAddress, 0 };
            index->saveToDisk(cursor, nodeSize, cursorAddress);
            return;
        }
    }

    if (leftSibling >= 0)
    {
        Node* leftNode = (Node*)index->loadFromDisk(parent->pointers[leftSibling], nodeSize);
        leftNode->keys[leftNode->curKeyCount] = parent->keys[leftSibling];

        for (int i = leftNode->curKeyCount + 1, j = 0; j < cursor->curKeyCount; j++)
        {
            leftNode->keys[i] = cursor->keys[j];
        }

        Address nullAddress{ nullptr, 0 };
        for (int i = leftNode->curKeyCount + 1, j = 0; j < cursor->curKeyCount + 1; j++)
        {
            leftNode->pointers[i] = cursor->pointers[j];
            cursor->pointers[j] = nullAddress;
        }

        leftNode->curKeyCount += cursor->curKeyCount + 1;
        cursor->curKeyCount = 0;
        index->saveToDisk(leftNode, nodeSize, parent->pointers[leftSibling]);
        removeInternal(parent->keys[leftSibling], (Node*)parentDiskAddress, (Node*)cursorDiskAddress);
    }
    else if (rightSibling <= parent->curKeyCount)
    {
        Node* rightNode = (Node*)index->loadFromDisk(parent->pointers[rightSibling], nodeSize);
        cursor->keys[cursor->curKeyCount] = parent->keys[rightSibling - 1];

        for (int i = cursor->curKeyCount + 1, j = 0; j < rightNode->curKeyCount; j++)
        {
            cursor->keys[i] = rightNode->keys[j];
        }

        Address nullAddress = { nullptr, 0 };
        for (int i = cursor->curKeyCount + 1, j = 0; j < rightNode->curKeyCount + 1; j++)
        {
            cursor->pointers[i] = rightNode->pointers[j];
            rightNode->pointers[j] = nullAddress;
        }

        cursor->curKeyCount += rightNode->curKeyCount + 1;
        rightNode->curKeyCount = 0;

        Address cursorAddress{ cursorDiskAddress, 0 };
        index->saveToDisk(cursor, nodeSize, cursorAddress);
        void* rightNodeAddress = parent->pointers[rightSibling].blockAddress;
        removeInternal(parent->keys[rightSibling - 1], (Node*)parentDiskAddress, (Node*)rightNodeAddress);
    }
}

void BPlusTree::removeLL(Address LLHeadAddress)
{
    Node* head = (Node*)index->loadFromDisk(LLHeadAddress, nodeSize);
    index->deallocate(LLHeadAddress, nodeSize);

    if (head->pointers[head->curKeyCount].blockAddress == nullptr)
    {
        std::cout << "End of linked list";
        return;
    }

    if (head->pointers[head->curKeyCount].blockAddress != nullptr)
    {
        removeLL(head->pointers[head->curKeyCount]);
    }
}
