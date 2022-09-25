#include "bplustree.h"
#include "types.h"

#include <iostream>

using namespace std;

// Search operation
void BPlusTree::search(float lowerBound, float upperBound)
{
  if (root == nullptr)
  {
    cout << "Tree is empty\n";
    return;
  }
  else
  {
    // Load in root from disk.
    Address rootDiskAddress{rootAddress, 0};
    root = (Node *)index->loadFromDisk(rootDiskAddress, nodeSize);

    // output
    cout << "Accessing index node. Displaying node: ";
    displayNode(root);

    Node *cursor = root;

    // Loop until we reach a leaf node
    while (cursor->isLeaf == false)
    {
      // Iterate through the keys in the current node
      for (int i = 0; i < cursor->curKeyCount; i++)
      {

        if (lowerBound < cursor->keys[i])
        {
          // Load node from disk to main memory.
          cursor = (Node *)index->loadFromDisk(cursor->pointers[i], nodeSize);
          // output
          cout << "Accessing index node. Displaying node: ";
          displayNode(cursor);
          break;
        }

        // If there are no more keys in this node,
        if (i == cursor->curKeyCount - 1)
        {
          // Load node from disk to main memory.
          // Set cursor to the child node, now loaded in main memory.
          cursor = (Node *)index->loadFromDisk(cursor->pointers[i + 1], nodeSize);

          // for displaying to output file
          cout << "Accessing index node. Displaying node: ";
          displayNode(cursor);
          break;
        }
      }
    }

    // We've reach a leaf node (i.e. the 1st level)
    // Iterate through all the nodes until upper bound or out of range
    bool done = false;
    while (!done)
    {

      int i = 0;
      for (i = 0; i < cursor->curKeyCount; i++)
      {
        // key exceeds Upper bound
        if (cursor->keys[i] > upperBound)
        {
          done = true;
          break;
        }

        // key is still within range of lower bound and upper bound
        if (cursor->keys[i] >= lowerBound && cursor->keys[i] <= upperBound)
        {
          // output
          cout << "Accessing leaf node. Displaying node: ";
          displayNode(cursor);

          cout << endl;
          cout << "Average rating: " << cursor->keys[i] << " > ";

          // Access the LL and print to output
          displayLL(cursor->pointers[i]);
        }
      }

      // Check if the last key is still within range of lower bound and upper bound. If so, move to next leaf node
      if (cursor->keys[i] != upperBound && cursor->pointers[cursor->curKeyCount].blockAddress != nullptr)
      {
        // Set cursor to be next leaf node
        cursor = (Node *)index->loadFromDisk(cursor->pointers[cursor->curKeyCount], nodeSize);

        // output
        cout << "Accessing leaf node. Displaying node: ";
        displayNode(cursor);
      }
      else
      {
        done = true;
      }
    }

  }
  return;
}