#include "bplustree.h"
#include "types.h"

#include <iostream>
#include <cstring>

using namespace std;

void BPlusTree::displayNode(Node *btreenode)
{
  // Print out all contents in the node as such |pointer|key|pointer|
  int ctr = 0;
  std::cout << ">";
  int keys = btreenode->numKeys;
  while (ctr < keys)
  {
    std::cout << btreenode->pointers[ctr].blockAddress << " > ";
    std::cout << btreenode->keys[ctr] << " > ";
    ctr++;
  }

  ctr++;
  // Print last filled pointer
  string lastptr = btreenode->pointers[ctr].blockAddress == nullptr ? "none" : ">";
  std::cout << lastptr;

  // for (int ctr = node->numKeys; ctr < maxKeys; ctr++)
  // {
  //   std::cout << " x >";      // Remaining empty keys
  //   std::cout << "  Null  >"; // Remaining empty pointers
  // }

  // std::cout << endl;
}

// Display a block and its contents in the disk. Assume it's already loaded in main memory.
void BPlusTree::displayBlock(void *blkAdr)
{
  // Load block into memory
  void *blk = operator new(nodeSize);
  std::memcpy(blk, blkAdr, nodeSize);

  unsigned char blkchk[nodeSize];
  memset(blkchk, '\0', nodeSize);

  // Block is empty.
  if (memcmp(blkchk, blk, nodeSize) == 0)
  {
    std::cout << "empty" << '\n';
    return;
  }

  unsigned char *blkchar = (unsigned char *)blk;

  int ctr = 0;
  while (ctr < nodeSize)
  {
    // Load each record
    void *recAdr = operator new(sizeof(Record));
    std::memcpy(recAdr, blkchar, sizeof(Record));
    Record *rec = (Record *)recAdr;

    std::cout << "[" << rec->tconst << "|" << rec->averageRating << "|" << rec->numVotes << "]  ";

    blkchar += sizeof(Record);
    ctr += sizeof(Record);
  }
}

// Print the tree
void BPlusTree::display(Node *flagDisk, int lvl)
{
  Address flagMM{flagDisk, 0};
  Node *flag = (Node *)index->loadFromDisk(flagMM, nodeSize);

  // If tree exists, display all nodes.
  if (flag != nullptr)
  {
    int ctr = 0;
    while (ctr < lvl)
    {
      std::cout << ' ';
      ctr++;
    }

    std::cout << " lvl " << lvl << "= ";
    ctr = 0;

    displayNode(flag);

    if (flag->isLeaf == false)
    {
      while (ctr < flag->numKeys + 1)
      {
        Node *mmNode = (Node *)index->loadFromDisk(flag->pointers[ctr], nodeSize);
        display((Node *)mmNode, lvl + 1);
        ctr++;
      }
    }
  }
}

void BPlusTree::displayLL(Address llHead)
{
  // Load linked list head into main memory.
  Node *head = (Node *)index->loadFromDisk(llHead, nodeSize);

  // Print all records in the linked list.
  int ctr = 0;
  while (ctr < head->numKeys)
  {
    std::cout << "\ndata block";
    displayBlock(head->pointers[ctr].blockAddress);
    std::cout << endl;

    Record res = *(Record *)(disk->loadFromDisk(head->pointers[ctr], sizeof(Record)));
    std::cout << res.tconst << " | ";

    ctr++;
  }

  ctr++;
  // Print empty slots
  while (ctr < maxKeys)
  {
    std::cout << "empty";
    ctr++;
  }

  // End of linked list or otherwise
  if (head->pointers[head->numKeys].blockAddress == nullptr)
  {
    std::cout << "ll end" << endl;
    return;
  }
  else
  {
    displayLL(head->pointers[head->numKeys]);
  }
}