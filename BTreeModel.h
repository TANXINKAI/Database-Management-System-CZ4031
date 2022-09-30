// B+ tree in C++ with insertion, search, rangequery, deletion and rebuild. 

#include <climits>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <vector>
#include "Storage.h"
using namespace std;

int MAX_KEYS = 3;
class BplusTree;
class Node
{
	bool IS_LEAFNODE;
	int *key, size;
	Node **ptr;
	int *addressBlock;
	int *addressOffset;
	friend class BplusTree;

public:
	Node();
	bool isLeaf()
	{
		return IS_LEAFNODE;
	}
	int getSize()
	{
		return this->size;
	}
	int *getKeys()
	{
		return this->key;
	}
	int *getBlocks()
	{
		return this->addressBlock;
	}
	int *getOffsets()
	{
		return this->addressOffset;
	}
	Node **getPointers()
	{
		return this->ptr;
	}
};
class BplusTree
{
	Node *root;
	int keyCount = 0;
	int height = 0; // height of tree, n = MAX_KEYS = 3
	int no_nodes;	// number of nodes in the tree

	void insertInternal(int, int, int, Node *, Node *);
	void removeInternal(int, Node *, Node *);
	Node *findParent(Node *, Node *);

public:
	BplusTree();
	BplusTree(int n);
	Node *getRoot();
	int getTreeOrder();
	int getHeight();

	void search(int);
	void rangequery(int, int);
	void insert(int, int, int);
	int remove(int);
	int count_memory(Node *);
	int count_nodes(Node *);
	void display(Node *);
	void leveldisplay(Node *, int, vector<string> *);
	void leafNodedisplay(Node *);
	void leafNodedisplaylimited(Node *, int);

	void rebuild(Node *, int , int);

	Storage *storage = nullptr;
};
Node::Node()
{
	key = new int[MAX_KEYS]; // creating an array of length N/MAX_KEYS (Each node should have N keys)
	addressBlock = new int[MAX_KEYS];
	addressOffset = new int[MAX_KEYS];
	ptr = new Node *[MAX_KEYS + 1]; // Whats the purpose of this? i thought the keys within the same node is in an array... if use ptr, shouldnt key = new Node[MAX_KEYS] or smth?
	// MAX_KEYS + 1 because MAX_KEYS = number of keys. Pointer count = number of keys + 1. Hence MAX_KEYS + 1
	for (int i = 0; i < MAX_KEYS + 1; i++) // Initialize all ptrs to nullptr. Uninitialized pointers are problematic when trying to check for validity.
		ptr[i] = nullptr;
}
BplusTree::BplusTree(int n)
{
	root = nullptr;
	MAX_KEYS = n;
}
BplusTree::BplusTree()
{
}

Node *BplusTree::getRoot()
{
	return root;
}

int BplusTree::getTreeOrder()
{
	return MAX_KEYS;
}

int BplusTree::getHeight()
{
	return this->height;
}

void BplusTree::insert(int x, int block, int offset)
{
	this->keyCount++;
	if (root == nullptr)
	{
		root = new Node;
		root->key[0] = x;
		root->addressBlock[0] = block;
		root->addressOffset[0] = offset;
		root->IS_LEAFNODE = true;
		root->size = 1; // size of root node = 1
		this->height = 1;
	}
	else
	{
		Node *curr = root;
		Node *parent = nullptr;
		while (!curr->IS_LEAFNODE)
		{
			parent = curr;
			for (int i = 0; i < curr->size; i++)
			{
				if (x <= curr->key[i])
				{
					curr = curr->ptr[i];
					break;
				}
				if (i == curr->size - 1)
				{
					curr = curr->ptr[i + 1];
					break;
				}
			}
		}
		if (curr->size < MAX_KEYS)
		{
			int i = 0;
			while (x > curr->key[i] && i < curr->size) // Set i to idx of first idx with val higher than x
				i++;

			for (int j = curr->size; j > i && j > 0; j--) // Shift all keys larger than x to the right by 1
			{
				curr->key[j] = curr->key[j - 1];
				curr->addressBlock[j] = curr->addressBlock[j - 1];
				curr->addressOffset[j] = curr->addressOffset[j - 1];
			}
			curr->key[i] = x;
			curr->addressBlock[i] = block;
			curr->addressOffset[i] = offset;
			curr->size++; // Increase size of Node after each "insertion" to the key array

			// Shift pointer right by 1 (for inserting keys that are sandwiched in between pointers)
			curr->ptr[curr->size] = curr->ptr[curr->size - 1];
			curr->ptr[curr->size - 1] = nullptr;
		}
		else // If current curr node size == N/MAX_KEYS Keys in Node
		{
			Node *newLeaf = new Node;
			int *tempNode = new int[MAX_KEYS + 1];
			int *tempNodeBlock = new int[MAX_KEYS + 1];
			int *tempNodeOffset = new int[MAX_KEYS + 1];
			for (int i = 0; i < curr->size; i++)
			{
				tempNode[i] = curr->key[i];
				tempNodeBlock[i] = curr->addressBlock[i];
				tempNodeOffset[i] = curr->addressOffset[i];
			}
			int i = 0, j;
			while (x > tempNode[i] && i < MAX_KEYS)
				i++;
			for (int j = MAX_KEYS; j > i && j > 0; j--)
			{
				tempNode[j] = tempNode[j - 1];
				tempNodeBlock[j] = tempNodeBlock[j - 1];
				tempNodeOffset[j] = tempNodeOffset[j - 1];
			}
			tempNode[i] = x;
			tempNodeBlock[i] = block;
			tempNodeOffset[i] = offset;
			newLeaf->IS_LEAFNODE = true;
			// floor taken
			newLeaf->size = MAX_KEYS + 1 - ((MAX_KEYS + 1) / 2);

			// newLeaf->size = curr->size + 1 - (MAX_KEYS + 1) / 2;

			// Point the new leaf's last pointer to the right siblings last pointer
			if (curr->ptr[curr->size] != nullptr)
				{
					newLeaf->ptr[newLeaf->size] = curr->ptr[curr->size];
				}

			curr->size = (MAX_KEYS + 1) / 2;
			curr->ptr[curr->size] = newLeaf;

			curr->ptr[MAX_KEYS] = nullptr;
			for (i = 0; i < MAX_KEYS + 1; i++)
			{
				if (i < curr->size)
				{
					curr->key[i] = tempNode[i];
					curr->addressBlock[i] = tempNodeBlock[i];
					curr->addressOffset[i] = tempNodeOffset[i];
				}
				else
				{
					newLeaf->key[i - curr->size] = tempNode[i];
					newLeaf->addressBlock[i - curr->size] = tempNodeBlock[i];
					newLeaf->addressOffset[i - curr->size] = tempNodeOffset[i];
				}
			}
			if (curr == root)
			{
				Node *newRoot = new Node;
				newRoot->key[0] = newLeaf->key[0];
				newRoot->addressBlock[0] = newLeaf->addressBlock[0];
				newRoot->addressOffset[0] = newLeaf->addressOffset[0];
				newRoot->ptr[0] = curr;
				newRoot->ptr[1] = newLeaf;
				newRoot->IS_LEAFNODE = false;
				newRoot->size = 1;
				root = newRoot;
				this->height++;
			}
			else
			{
				insertInternal(newLeaf->key[0], newLeaf->addressBlock[0], newLeaf->addressOffset[0], parent, newLeaf); // create a new internal node in B+ tree
			}
			free(tempNode);
			free(tempNodeBlock);
			free(tempNodeOffset);
		}
	}
}

// Search operation
void BplusTree::search(int x)
{
	vector<string> contents;
	int nodesAccessed = 1;
	bool keyFound = false;
	if (root == nullptr)
	{
		std::cout << "Tree is empty\n";
	}
	else
	{
		Node *curr = root;
		while (curr->IS_LEAFNODE == false)
		{
			nodesAccessed += 1;
			string strKeys = "";
			int *keys = curr->getKeys();
			double rating = 0.0;

			for (int i = 0; i < curr->size; i++)
			{
				rating += storage->getMovieInfoAt(curr->addressBlock[i], curr->addressOffset[i]).getRating();
				strKeys.append(to_string(keys[i]) + " ");
			}
			strKeys.append("(Average rating: " + to_string(rating / curr->size) + ")");
			contents.push_back(strKeys);
			for (int i = 0; i < curr->size; i++)
			{
				if (x <= curr->key[i])
				{
					curr = curr->ptr[i];
					break;
				}
				if (i == curr->size - 1)
				{
					curr = curr->ptr[i + 1];
					break;
				}
			}
		}
		string strKeys = "";
		int *keys = curr->getKeys();
		double rating = 0.0;
		for (int i = 0; i < curr->size; i++)
		{
			rating += storage->getMovieInfoAt(curr->addressBlock[i], curr->addressOffset[i]).getRating();
			strKeys.append(to_string(keys[i]) + " ");
		}
		strKeys.append("(Average rating: " + to_string(rating / curr->size) + ")");
		contents.push_back(strKeys);
		std::cout << to_string(nodesAccessed) << " nodes accessed during search for key '" << to_string(x) << "'" << endl;
		for (int i = 0; i < (nodesAccessed > 5 ? 5 : nodesAccessed); i++)
		{
			std::cout << "Node " << to_string(i + 1) << " keys: " << contents[i] << endl;
		}

		while (curr->key[0] <= x)
		{
			for (int i = 0; i < curr->size; i++)
			{
				// std::cout << "key is : " << curr->key[i] << "\t curr->size: " << curr->getSize() << endl;
				if (curr->key[i] == x)
				{
					// std::cout << "Found key " << to_string(x) << "\t tconst: " << storage->getMovieInfoAt(curr->addressBlock[i], curr->addressOffset[i]).getTConst() << "\n";
					if (!keyFound)
					{
						keyFound = true;
					}
				}
			}
			//end of leaf nodes.
			if (!curr->ptr[curr->size])
			{
				std::cout << "terminated by null" << endl;
				free(curr);
				return;
			}
			curr = curr->ptr[curr->size];
			// std::cout << "new curr->key[curr->size]: " << curr->key[curr->size] << endl;
		}

		if (!keyFound)
				std::cout << "Could not find key " << to_string(x) << endl;
		return;
	}
}

// Ranged Search operation
void BplusTree::rangequery(int lb, int hb)
{
	vector<string> contents;
	vector<int> blocksAccessedList;
	int nodesAccessed = 1;
	if (root == nullptr)
	{
		std::cout << "Tree is empty\n";
	}
	else
	{
		Node *curr = root;
		while (curr->IS_LEAFNODE == false)
		{
			nodesAccessed += 1;
			string strKeys = "";
			int *keys = curr->getKeys();
			double rating = 0.0;
			for (int i = 0; i < curr->size; i++)
			{
				rating += storage->getMovieInfoAt(curr->addressBlock[i], curr->addressOffset[i]).getRating();
				bool unique = true;
				for (int j = 0; j < blocksAccessedList.size(); j++)
					if (blocksAccessedList[j] == curr->addressBlock[i])
					{
						unique = false;
						break;
					}
				if (unique)
					blocksAccessedList.push_back(curr->addressBlock[i]);
				strKeys.append(to_string(keys[i]) + " ");
			}
			strKeys.append("(Average rating: " + to_string(rating / curr->size) + ")");
			contents.push_back(strKeys);
			for (int i = 0; i < curr->size; i++)
			{
				if (lb <= curr->key[i])
				{
					curr = curr->ptr[i];
					break;
				}
				if (i == curr->size - 1)
				{
					curr = curr->ptr[i + 1];
					break;
				}
			}
		}
		string strKeys = "";
		int *keys = curr->getKeys();
		double rating = 0.0;
		for (int i = 0; i < curr->size; i++)
		{
			rating += storage->getMovieInfoAt(curr->addressBlock[i], curr->addressOffset[i]).getRating();
			bool unique = true;
			for (int j = 0; j < blocksAccessedList.size(); j++)
				if (blocksAccessedList[j] == curr->addressBlock[i])
				{
					unique = false;
					break;
				}
			if (unique)
				blocksAccessedList.push_back(curr->addressBlock[i]);
			strKeys.append(to_string(keys[i]) + " ");
		}
		strKeys.append("(Average rating: " + to_string(rating / curr->size) + ")");
		contents.push_back(strKeys);
		std::cout << to_string(nodesAccessed) << " nodes accessed during search for key in range ('" << to_string(lb) << "','" << to_string(hb) << "')" << endl;
		std::cout << to_string(blocksAccessedList.size()) << " blocks accessed during search for key in range ('" << to_string(lb) << "','" << to_string(hb) << "')" << endl;
		for (int i = 0; i < (nodesAccessed > 5 ? 5 : nodesAccessed); i++)
		{
			std::cout << "Node " << to_string(i + 1) << " keys: " << contents[i] << endl;
		}

		// loop for values in range.
		bool keyFound = false;

		if (curr == nullptr)
			return;

		// take leftmost pointer to obtain leafnode.
		while (curr->IS_LEAFNODE == false)
		{
			int *keys = curr->getKeys();
			std::cout << keys[0] << endl;
			curr = curr->ptr[0];
		}

		while (curr->key[0] <= hb)
		{
			// std::cout << "curr->size : " << curr->size << endl;
			for (int i = 0; i < curr->size; i++)
			{
				if (curr->key[i] < lb || curr->key[i] > hb)
				{
					// std::cout << "lb / hb skipped" << endl;
					continue;
				}
				else
				{
					if (!keyFound)
					{
						keyFound = true;
					}
					// std::cout << "Found key " << curr->key[i] << "\t tconst: " << storage->getMovieInfoAt(curr->addressBlock[i], curr->addressOffset[i]).getTConst() << "\n";
				}
			}
			if (!curr->ptr[curr->size])
			{
				std::cout << "terminated by null" << endl;
				free(curr);
				return;
			}
			curr = curr->ptr[curr->size];
			std::cout << endl;
		}

		if (!keyFound)
			std::cout << " Not found\n";
	}
}
void BplusTree::insertInternal(int x, int block, int offset, Node *curr, Node *child) // insert a new internal node in B+ tree
{
	if (curr->size < MAX_KEYS) // Mainly to update the Keys in parent's node
	{
		int i = 0;
		while (x > curr->key[i] && i < curr->size)
			i++;
		for (int j = curr->size + 1; j > i; j--)
		{
			if (j > i && j <= curr->size)
			{
				curr->key[j] = curr->key[j - 1];
				curr->addressBlock[j] = curr->addressBlock[j - 1];
				curr->addressOffset[j] = curr->addressOffset[j - 1];
			}
			curr->ptr[j] = curr->ptr[j - 1];
		}
		curr->key[i] = x;
		curr->addressBlock[i] = block;
		curr->addressOffset[i] = offset;
		curr->size++;
		curr->ptr[i + 1] = child;
	}
	else
	{
		Node *newInternal = new Node;
		int *tempKey = new int[MAX_KEYS + 1]; // temp key array. Will have N + 1 keys, need to split
		int *tempBlock = new int[MAX_KEYS + 1];
		int *tempOffset = new int[MAX_KEYS + 1];
		Node **tempPtr = (Node **)malloc(sizeof(Node) * (MAX_KEYS + 2));
		for (int i = 0; i < MAX_KEYS + 2; i++)
			tempPtr[i] = nullptr;
		for (int i = 0; i < curr->size + 1; i++) // (For Loop) is for Left sub tree? (Austin: No, this is for cloning the entire curr key and pointers into tempKey and tempPtr respectively)
		{
			if (i < curr->size)
			{
				tempKey[i] = curr->key[i];
				tempBlock[i] = curr->addressBlock[i];
				tempOffset[i] = curr->addressOffset[i];
			}
			tempPtr[i] = curr->ptr[i];
		}
		int i = 0, j;
		while (x > tempKey[i] && i < MAX_KEYS) // Retrieve the spot (index) of where we want to insert the key
			i++;
		for (int j = MAX_KEYS + 1; j > i; j--) // (For Loop) is for Right sub tree? (Austin: No, this is for shifting keys and pointers to the right (of index i))
		{
			if (j > 0 && j <= MAX_KEYS)
			{
				tempKey[j] = tempKey[j - 1];
				tempBlock[j] = tempBlock[j - 1];
				tempOffset[j] = tempOffset[j - 1];
			}
			tempPtr[j] = tempPtr[j - 1];
		}

		tempKey[i] = x;
		tempBlock[i] = block;
		tempOffset[i] = offset;
		tempPtr[i + 1] = child;
		newInternal->IS_LEAFNODE = false;
		curr->size = (MAX_KEYS + 1) / 2;
		newInternal->size = MAX_KEYS - (MAX_KEYS + 1) / 2;

		for (i = 0, j = curr->size + 1; i < newInternal->size + 1; i++, j++) // Assigning Left Sub tree? WHat about right sub tree? (Austin: This is for creating internal node, there is no left/right tree. See next 2 comments)
		{
			if (i < newInternal->size)
			{
				newInternal->key[i] = tempKey[j];
				newInternal->addressBlock[i] = tempBlock[j];
				newInternal->addressOffset[i] = tempOffset[j];
			}
			newInternal->ptr[i] = tempPtr[j];
		}
		free(tempKey);
		free(tempBlock);
		free(tempOffset);
		free(tempPtr);
		if (curr == root) // Need somewhere to Update the Height and Number of Nodes in B+ tree
		{
			Node *newRoot = new Node;
			newRoot->key[0] = curr->key[curr->size];
			newRoot->addressBlock[0] = curr->addressBlock[curr->size];
			newRoot->addressOffset[0] = curr->addressOffset[curr->size];
			// Austin: Point the internal node pointers to left and right sub tree accordingly
			newRoot->ptr[0] = curr;
			newRoot->ptr[1] = newInternal;
			newRoot->IS_LEAFNODE = false;
			newRoot->size = 1;
			root = newRoot;
			this->height++;
		}
		else // Update the parent Node's key
		{
			insertInternal(curr->key[curr->size], curr->addressBlock[curr->size], curr->addressOffset[curr->size], findParent(root, curr), newInternal);
		}
	}
}
Node *BplusTree::findParent(Node *curr, Node *child)
{
	Node *parent = nullptr;
	if (curr->IS_LEAFNODE || (curr->ptr[0])->IS_LEAFNODE)
	{
		return nullptr;
	}
	for (int i = 0; i < curr->size + 1; i++)
	{
		if (curr->ptr[i] == child)
		{
			parent = curr;
			return parent;
		}
		else
		{
			parent = findParent(curr->ptr[i], child);
			if (parent != nullptr)
				return parent;
		}
	}
	return parent;
}

int BplusTree::remove(int x)
{
	if (root == nullptr)
	{
		std::cout << "Tree empty\n";
		return -1;
	}
	else
	{
		Node *curr = root;
		Node *parent = nullptr;
		int leftSibling, rightSibling;
		while (curr->IS_LEAFNODE == false)
		{
			for (int i = 0; i < curr->size; i++)
			{
				parent = curr;
				// out of bound for i = 0, no need for shift.
				leftSibling = i - 1;
				rightSibling = i + 1;
				if (x < curr->key[i])
				{
					curr = curr->ptr[i];
					break;
				}
				if (i == curr->size - 1)
				{
					leftSibling = i;
					rightSibling = i + 2;
					curr = curr->ptr[i + 1];
					break;
				}
			}
		}
		bool found = false;
		int pos;
		for (pos = 0; pos < curr->size; pos++)
		{
			if (curr->key[pos] == x)
			{
				// std::cout << x <<"  Found and removed\n" <<endl;
				found = true;
				break;
			}
		}
		if (!found)
		{
			std::cout << x <<" Not found and deleted" << endl;
			return -1;
		}
		// shift position of keys after position of key to be removed.
		for (int i = pos; i < curr->size; i++)
		{
			curr->key[i] = curr->key[i + 1];
		}
		curr->size--;
		if (curr == root)
		{
			for (int i = 0; i < MAX_KEYS + 1; i++)
			{
				curr->ptr[i] = nullptr;
			}
			if (curr->size == 0)
			{
				std::cout << "Tree died\n";
				delete[] curr->key;
				delete[] curr->ptr;
				delete curr;
				root = nullptr;
			}
			return 1;
		}
		// assigning the last ptr to the next node's ptr
		curr->ptr[curr->size] = curr->ptr[curr->size + 1];
		curr->ptr[curr->size + 1] = nullptr;
		if (curr->size >= (MAX_KEYS + 1) / 2)
		{
		    // meets minimum size criteria
		    int nkey;
            nkey = curr->key[0];
			rebuild(root, x, nkey);
			return 1;
		}
		int nkey;
		if (leftSibling >= 0)
		{
			Node *leftNode = parent->ptr[leftSibling];
			if (leftNode->size >= (MAX_KEYS + 1) / 2 + 1)
			{
			    // borrow (largest) from left sibling if it can spare
				for (int i = curr->size; i > 0; i--)
				{
					curr->key[i] = curr->key[i - 1];
				}
				curr->size++;
				// assigning the last ptr to the next node's ptr
				curr->ptr[curr->size] = curr->ptr[curr->size - 1];
				curr->ptr[curr->size - 1] = nullptr;
				// borrow largest from left
				curr->key[0] = leftNode->key[leftNode->size - 1];
				leftNode->size--;
				// assigning left sibling last ptr to current node's ptr
				leftNode->ptr[leftNode->size] = curr;
				leftNode->ptr[leftNode->size + 1] = nullptr;
				// don't need to update the keys of left sibling because size--, any traversal will not reach it. Insertion will overwrite
				// on the parent node, edit the smallest number of current node
				parent->key[leftSibling] = curr->key[0];

				// rebuild for key deleted is updated scenario
				nkey = curr->key[0];
				rebuild(root, x, nkey);

				// rebuild for key not deleted (new first index) has to be updated scenario
				nkey = curr->key[0];
				rebuild(root, curr->key[1], nkey);

				return 1;
			}
		}
		if (rightSibling <= parent->size)
		{
			Node *rightNode = parent->ptr[rightSibling];

			if (rightNode->size >= (MAX_KEYS + 1) / 2 + 1)
			{
			    // borrow smallest from right sibling
				curr->size++;
				// curr-> size is the new size
				curr->key[curr->size - 1] = rightNode->key[0];
				// decrease size of rightNode
				rightNode->size--;
				// updating the rightNode's ptr after "borrowing"
				rightNode->ptr[0] = rightNode->ptr[1];
				for (int i = 0; i < rightNode->size; i++)
				{
					rightNode->key[i] = rightNode->key[i + 1];
				}
				// Assigning the last ptr to the rightNode ptr. 2nd last ptr don't need to reupdate as it is already pointed to the key of previous smallest rightNode key
				curr->ptr[curr->size] = rightNode;

				// rebuild for key deleted is updated scenario
				nkey = curr->key[0];
				rebuild(root, x, nkey);

				// rebuild for key not deleted (new first index) has to be updated scenario
				nkey = rightNode->key[0];
				rebuild(root, curr->key[curr->size -1], nkey);

				parent->key[rightSibling - 1] = rightNode->key[0];
				return 1;
			}
		}
		if (leftSibling >= 0)
		{
			Node *leftNode = parent->ptr[leftSibling];

			// rebuild for key merged updated scenario
			// nkey = leftNode->key[leftNode->size];
			// case 2
			int xtemp;
			xtemp = curr->key[0];

			for (int i = leftNode->size, j = 0; j < curr->size; i++, j++)
			{
			    // adding current node(right) content into left node content
				leftNode->key[i] = curr->key[j];	
				leftNode->ptr[i] = curr->ptr[j];
			}

			leftNode->ptr[leftNode->size] = nullptr; 
			leftNode->size += curr->size;
			// updating the merged node's ptr to the deleted node's last ptr
			leftNode->ptr[leftNode->size] = curr->ptr[curr->size];
			
			// removing current node
			removeInternal(parent->key[leftSibling], parent, curr);

			// rebuilding is done after all adjustments to leaf and its immediate parent...
			// merging case where deleted key is the most left key of the leaf node x=deleted node (Case 1), if not key[0] being deleted, its okay cause parent tree can never find it, dn rebuild
			// key that could also require update is the curr node(right) first key (Case 2), cause it will be "deleted"/merged
			// new key should be the updated left node first key
			nkey = leftNode->key[0];
			rebuild(root, x, nkey);

			//case 2 , see line 785 for xtemp. its the 1st key of the right node(curr)
			rebuild(root,xtemp,nkey);

			delete[] curr->key;
			delete[] curr->ptr;
			delete curr;
		}
		else if (rightSibling <= parent->size)
		{
			Node *rightNode = parent->ptr[rightSibling];

			// rebuild for key merged updated scenario (it should be the same as the above, since we are shifting right node to left node
            // then deleting the right node
			// case 2
			int xtemp;
			xtemp = rightNode->key[0];

			// Node *tempMergeNode = parent->ptr[leftSibling];
			for (int i = curr->size, j = 0; j < rightNode->size; i++, j++)
			{
			    // adding right node content into current node(left) content
				curr->key[i] = rightNode->key[j];
				// curr->ptr[i] = rightNode->ptr[j];
			}
			curr->ptr[curr->size] = nullptr;
			curr->size += rightNode->size;
			curr->ptr[curr->size] = rightNode->ptr[rightNode->size];

			// delete the right node
			removeInternal(parent->key[rightSibling - 1], parent, rightNode);

			// rebuilding is done after all adjustments to leaf and its immediate parent...
			// merging case where deleted key is the most left key of the leaf node x=deleted node (Case 1), if not key[0] being deleted, its okay cause parent tree can never find it, dn rebuild
			// key that could also require update is the curr node(right) first key (Case 2), cause it will be "deleted"/merged
			// new key should be the updated left node first key
			nkey = curr->key[0];
			rebuild(root, x, nkey);

			//case 2 , see line 825 for xtemp. its the 1st key of the right node
			rebuild(root,xtemp,nkey);

			delete[] rightNode->key;
			delete[] rightNode->ptr;
			delete rightNode;
		}
		// first index removed.
		return 1;
	}
}
void BplusTree::removeInternal(int x, Node *curr, Node *child)
{
	if (curr == root)
	{
		if (curr->size == 1)
		{
			// left child removed
			if (curr->ptr[0] == child)
			{
				delete[] child->key;
				delete[] child->ptr;
				delete child;
				root = curr->ptr[1];
				delete[] curr->key;
				delete[] curr->ptr;
				delete curr;
				return;
			}
			// right child removed
			else if (curr->ptr[1] == child)
			{
				delete[] child->key;
				delete[] child->ptr;
				delete child;
				root = curr->ptr[0];
				delete[] curr->key;
				delete[] curr->ptr;
				delete curr;
				return;
			}
		}
	}
	int pos;
	for (pos = 0; pos < curr->size; pos++)
	{
	    // finding "pos" of key deleted
		if (curr->key[pos] == x)
		{
			break;
		}
	}
	for (int i = pos; i < curr->size; i++)
	{
	    // updating key of the "parent" node
		curr->key[i] = curr->key[i + 1];
	}
	for (pos = 0; pos < curr->size + 1; pos++)
	{
	    // finding the "pos" of the pointer that is pointed to the deleted child
		if (curr->ptr[pos] == child)
		{
			break;
		}
	}
	for (int i = pos; i < curr->size + 1; i++)
	{
	    // updating the ptrs of the "parent" node
		curr->ptr[i] = curr->ptr[i + 1];
	}
	curr->size--;
	if (curr->size >= (MAX_KEYS + 1) / 2 - 1)
	{
	    // meets minimum key requirements
		return;
	}
	if (curr == root)
		return;
	Node *parent = findParent(root, curr);
	int leftSibling, rightSibling;
	for (pos = 0; pos < parent->size + 1; pos++)
	{
		if (parent->ptr[pos] == curr)
		{
			leftSibling = pos - 1;
			rightSibling = pos + 1;
			break;
		}
	}
    // should here onwards be the same as "remove" , maybe call "remove" function?
	if (leftSibling >= 0)
	{
		Node *leftNode = parent->ptr[leftSibling];
		if (leftNode->size >= (MAX_KEYS + 1) / 2)
		{
			for (int i = curr->size; i > 0; i--)
			{
				curr->key[i] = curr->key[i - 1];
			}
			curr->key[0] = parent->key[leftSibling];
			parent->key[leftSibling] = leftNode->key[leftNode->size - 1];
			for (int i = curr->size + 1; i > 0; i--)
			{
				curr->ptr[i] = curr->ptr[i - 1];
			}
			curr->ptr[0] = leftNode->ptr[leftNode->size];
			curr->size++;
			leftNode->size--;
			return;
		}
	}
	if (rightSibling <= parent->size)
	{
		Node *rightNode = parent->ptr[rightSibling];
		if (rightNode->size >= (MAX_KEYS + 1) / 2)
		{
			curr->key[curr->size] = parent->key[pos];
			parent->key[pos] = rightNode->key[0];
			for (int i = 0; i < rightNode->size - 1; i++)
			{
				rightNode->key[i] = rightNode->key[i + 1];
			}
			curr->ptr[curr->size + 1] = rightNode->ptr[0];
			for (int i = 0; i < rightNode->size; ++i)
			{
				rightNode->ptr[i] = rightNode->ptr[i + 1];
			}
			curr->size++;
			rightNode->size--;
			return;
		}
	}
	if (leftSibling >= 0)
	{
		Node *leftNode = parent->ptr[leftSibling];
		leftNode->key[leftNode->size] = parent->key[leftSibling];
		for (int i = leftNode->size + 1, j = 0; j < curr->size; j++)
		{
			leftNode->key[i] = curr->key[j];
		}
		for (int i = leftNode->size + 1, j = 0; j < curr->size + 1; j++)
		{
			leftNode->ptr[i] = curr->ptr[j];
			curr->ptr[j] = nullptr;
		}
		leftNode->size += curr->size + 1;
		curr->size = 0;
		removeInternal(parent->key[leftSibling], parent, curr);
	}
	else if (rightSibling <= parent->size)
	{
		Node *rightNode = parent->ptr[rightSibling];
		curr->key[curr->size] = parent->key[rightSibling - 1];
		for (int i = curr->size + 1, j = 0; j < rightNode->size; j++)
		{
			curr->key[i] = rightNode->key[j];
		}
		for (int i = curr->size + 1, j = 0; j < rightNode->size + 1; j++)
		{
			curr->ptr[i] = rightNode->ptr[j];
			rightNode->ptr[j] = nullptr;
		}
		curr->size += rightNode->size + 1;
		rightNode->size = 0;
		removeInternal(parent->key[rightSibling - 1], parent, rightNode);
	}
}


void BplusTree::rebuild(Node *curr, int oldKey, int nKey)
{
	// update affected parent keys

	// std::cout<< "rebuild : " <<endl;
	// std::cout<< "\toldKey"<< oldKey <<"\tnkey:" << nKey <<endl;

	while (curr->IS_LEAFNODE == false)
		{
			for (int i = 0; i < curr->size; i++)
			{
				if (oldKey <= curr->key[i])
				{
					if (curr->key[i] == oldKey)
					{
						// std::cout << " curr->key[i] == oldKey " << root->size << endl;
						curr->key[i] = nKey;
					}
					curr = curr->ptr[i];
					break;
				}
				if (i == curr->size - 1)
				{
					curr = curr->ptr[i + 1];
					break;
				}
			}
		}
}


int BplusTree::count_memory(Node *curr)
{
	int retVal = 1; // As long as we enter this method, count as 1 as we are iterating a node to be in this method (this is necessary to account for internal nodes).

	if (curr == nullptr) // Edge case
		return 0;

	if (!curr->IS_LEAFNODE)
	{
		// If it is not a leaf node, we recurse through all available child nodes
		for (int i = 0; i < curr->size + 1; i++)
		{
			retVal = sizeof(curr);
			retVal += count_memory(curr->ptr[i]);
		}
	}
	return retVal;
}

// Not sure why it is not working (Austin: Fixed)
int BplusTree::count_nodes(Node *curr)
{
	int retVal = 1; // As long as we enter this method, count as 1 as we are iterating a node to be in this method (this is necessary to account for internal nodes).

	if (curr == nullptr) // Edge case
		return 0;

	if (!curr->IS_LEAFNODE)
	{
		// If it is not a leaf node, we recurse through all available child nodes
		for (int i = 0; i < curr->size + 1; i++)
			retVal += count_nodes(curr->ptr[i]);
	}
	return retVal;
}
void BplusTree::display(Node *curr)
{
	// Create a vector (array) of strings. 1 for each height of B+ tree
	vector<string> lines;
	for (int i = 0; i < this->height; i++)
		lines.push_back("");
	leveldisplay(curr, 0, &lines);

	// Best effort beautify print of each level
	for (int i = 0; i < this->height; i++)
	{
		for (int z = this->height - i - 1; z > 0; z--)
			std::cout << "            ";
		std::cout << lines[i] << endl;
	}
}

void BplusTree::leveldisplay(Node *curr, int level, vector<string> *txtOutput)
{
	for (int i = 0; i < curr->size; i++)
	{
		(*txtOutput)[level] = (*txtOutput)[level].append(to_string(curr->key[i]) + " ");
	}
	(*txtOutput)[level] = (*txtOutput)[level].append("    ");

	if (curr->IS_LEAFNODE != true)
	{
		for (int i = 0; i < curr->size + 1; i++)
		{
			leveldisplay(curr->ptr[i], level + 1, txtOutput);
			(*txtOutput)[level] = (*txtOutput)[level].append("    ");
		}
	}
}

void BplusTree::leafNodedisplay(Node *root){
	Node *temp = root;
	if(temp == nullptr) return;
	std::cout<<"leafNode display"<<endl;

	while (temp->IS_LEAFNODE == false){
		int *keys = temp->getKeys();
		std::cout<< keys[0] <<endl;
		temp = temp->ptr[0];
	}

	std::cout<<"is leaf node" << temp->IS_LEAFNODE <<endl;

	if(temp->key){
		std::cout<<temp->key[0]<<"is null"<<endl;
	}

	while (temp->key[0])
	{
		std::cout<<temp->size<<endl;
		for (int i = 0; i < temp->size; i++)
		{
			if (temp->key[i])
			{
				cout << temp->key[i] << "\t";
			}
			else{
				return;
			}
		}
		if(!temp->ptr[temp->size]) {
			std::cout<<"terminated by null"<<endl;
			free(temp);
			return;
			}
		temp = temp->ptr[temp->size];
		std::cout<<endl;
	}

}

void BplusTree::leafNodedisplaylimited(Node *root, int range){
	Node *temp = root;
	if(temp == nullptr) return;
	std::cout<<"leafNode display limited"<<endl;

	while (temp->IS_LEAFNODE == false){
		int *keys = temp->getKeys();
		std::cout<< keys[0] <<endl;
		temp = temp->ptr[0];
	}

	std::cout<<"is leaf node" << temp->IS_LEAFNODE <<endl;

	if(temp->key){
		std::cout<<temp->key[0]<<"is null"<<endl;
	}

	while (temp->key[0] && range >= 0)
	{
		std::cout<<temp->size<<endl;
		for (int i = 0; i < temp->size; i++)
		{
			if (temp->key[i])
			{
				cout << temp->key[i] << "\t";
			}
			else{
				return;
			}
		}
		if(!temp->ptr[temp->size]) {
			std::cout<<"terminated by null"<<endl;
			free(temp);
			return;
			}
		temp = temp->ptr[temp->size];
		std::cout<<endl;
		range--;
	}

}
