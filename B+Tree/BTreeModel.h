// B+ tree in C++ with deletion, insertion and search (With no ranged query)

#include <climits>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <vector>
using namespace std;
const int MAX = 3;

class BPTree;
class Node
{
	bool IS_LEAF;
	int *key, size;
	Node **ptr;
	friend class BPTree;

public:
	Node();
};
class BPTree
{
	Node *root;
	int keyCount = 0;
	int height = 0; // height of tree, n = Max = 3
	int no_nodes; // number of nodes in the tree

	void insertInternal(int, Node *, Node *);
	void removeInternal(int, Node *, Node *);
	Node *findParent(Node *, Node *);

public:
	BPTree();
	void search(int);
	void rangequery(int, int);
	void insert(int);
	void remove(int);
	void display(Node *);
	void display(Node *, int, vector<string>*);
	Node *getRoot();
	int count_nodes(Node *);
};
Node::Node()
{
	key = new int[MAX];   // creating an array of length N/Max (Each node should have N keys)
	ptr = new Node *[MAX+2];   // Whats the purpose of this? i thought the keys within the same node is in an array... if use ptr, shouldnt key = new Node[MAX] or smth?
}
BPTree::BPTree()
{
	root = nullptr;
}
void BPTree::insert(int x)
{
	this->keyCount++;
	if (root == nullptr)
	{
		root = new Node;
		root->key[0] = x;
		root->IS_LEAF = true;
		root->size = 1; // size of root node = 1
		this->height = 1;
	}
	else
	{
		Node *cursor = root;
		Node *parent = nullptr;
		while (!cursor->IS_LEAF)
		{
			parent = cursor;
			for (int i = 0; i < cursor->size; i++)
			{
				if (x < cursor->key[i])
				{
					cursor = cursor->ptr[i];
					break;
				}
				if (i == cursor->size - 1)
				{
					cursor = cursor->ptr[i + 1];
					break;
				}
			}
		}
		if (cursor->size < MAX)
		{
			int i = 0;
			while (x > cursor->key[i] && i < cursor->size) //Set i to idx of first idx with val higher than x
				i++;

			for (int j = cursor->size; j > i; j--) //Shift all keys larger than x to the right by 1
				cursor->key[j] = cursor->key[j - 1];
			cursor->key[i] = x;
			cursor->size++;   // Increase size of Node after each "insertion" to the key array


			//Austin: Lol what does this even do. Assigning empty pointers to other empty pointers
			//cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
			//cursor->ptr[cursor->size - 1] = NULL;
		}
		else // If current cursor node size == N/Max Keys in Node
		{
			Node *newLeaf = new Node;
			int virtualNode[MAX + 1];
			for (int i = 0; i < MAX; i++)
			{
				virtualNode[i] = cursor->key[i];
			}
			int i = 0, j;
			while (x > virtualNode[i] && i < MAX)
				i++;
			for (int j = MAX + 1; j > i; j--)
			{
				virtualNode[j] = virtualNode[j - 1];
			}
			virtualNode[i] = x;
			newLeaf->IS_LEAF = true;
			cursor->size = (MAX + 1) / 2;
			newLeaf->size = MAX + 1 - (MAX + 1) / 2;
			cursor->ptr[cursor->size] = newLeaf;

			// Austin: ? cursor->ptr[MAX] isn't the pointer pointing to next leaf, not sure what this is for.
			//newLeaf->ptr[newLeaf->size] = cursor->ptr[MAX];
			//cursor->ptr[MAX] = NULL;
			for (i = 0; i < MAX + 1; i++)
			{
				if(i < cursor->size)
					cursor->key[i] = virtualNode[i];
				else
					newLeaf->key[i - cursor->size] = virtualNode[i];
			}

			if (cursor == root)
			{
				Node *newRoot = new Node;
				newRoot->key[0] = newLeaf->key[0];
				newRoot->ptr[0] = cursor;
				newRoot->ptr[1] = newLeaf;
				newRoot->IS_LEAF = false;
				newRoot->size = 1;
				root = newRoot;
				this->height++;
			}
			else
			{
				insertInternal(newLeaf->key[0], parent, newLeaf); // create a new internal node in B+ tree
			}
		}
	}
}
// Search operation
void BPTree::search(int x)
{
	if (root == nullptr)
	{
		cout << "Tree is empty\n";
	}
	else
	{
		Node *cursor = root;
		while (cursor->IS_LEAF == false)
		{
			for (int i = 0; i < cursor->size; i++)
			{
				if (x < cursor->key[i])
				{
					cursor = cursor->ptr[i];
					break;
				}
				if (i == cursor->size - 1)
				{
					cursor = cursor->ptr[i + 1];
					break;
				}
			}
		}
		for (int i = 0; i < cursor->size; i++)
		{
			if (cursor->key[i] == x)
			{
				cout << " Found\n";
				return;
			}
		}
		cout << " Not found\n";
	}
}
// Ranged Search operation
void BPTree::rangequery(int lb, int hb)
{
	if (root == nullptr)
	{
		cout << "Tree is empty\n";
	}
	else
	{
		Node *cursor = root;
		while (cursor->IS_LEAF == false)
		{
			for (int i = 0; i < cursor->size; i++)
			{
				if (lb < cursor->key[i])
				{
					cursor = cursor->ptr[i];
					break;
				}
				if (i == cursor->size - 1)
				{
					cursor = cursor->ptr[i + 1];
					break;
				}
			}
		}
		for (int i = 0; i < cursor->size; i++)
		{
			if (cursor->key[i] == lb)
			{
				for (int j = i; j < cursor->size; j++)
				{
					if (cursor->key[j] > hb)
					{
						return;
					}
					{
						cout << cursor->key[j] << " Found\n";
					}

				}
				return;
			}
		}
		cout << " Not found\n";
	}
}
void BPTree::insertInternal(int x, Node *cursor, Node *child) // insert a new internal node in B+ tree
{
	if (cursor->size < MAX) // Mainly to update the Keys in parent's node
	{
		int i = 0;
		while (x > cursor->key[i] && i < cursor->size)
			i++;
		for (int j = cursor->size; j > i + 1; j--)
		{
			if(j < i)
				cursor->key[j] = cursor->key[j - 1];
			cursor->ptr[j] = cursor->ptr[j - 1];
		}
		cursor->key[i] = x;
		cursor->size++;
		cursor->ptr[i + 1] = child;
	}
	else
	{
		Node *newInternal = new Node;
		int virtualKey[MAX + 1];  // temp key array. Will have N + 1 keys, need to split
		Node *virtualPtr[MAX + 2];
		for (int i = 0; i < MAX + 2; i++) // (For Loop) is for Left sub tree? (Austin: No, this is for cloning the entire cursor key and pointers into virtualKey and virtualPtr respectively)
		{
			if(i < MAX)
				virtualKey[i] = cursor->key[i];
			virtualPtr[i] = cursor->ptr[i];
		}
		int i = 0, j;
		while (x > virtualKey[i] && i < MAX) //Retrieve the spot (index) of where we want to insert the key
			i++;

		for (int j = MAX + 1; j > i; j--) // (For Loop) is for Right sub tree? (Austin: No, this is for shifting keys and pointers to the right (of index i))
		{
			if (i < MAX)
				virtualKey[j] = virtualKey[j - 1];
			virtualPtr[j] = virtualPtr[j - 1];
		}

		virtualKey[i] = x;
		virtualPtr[i + 1] = child;
		newInternal->IS_LEAF = false;
		cursor->size = (MAX + 1) / 2;
		newInternal->size = MAX - (MAX + 1) / 2;

		for (i = 0, j = cursor->size + 1; i < newInternal->size + 2; i++, j++) // Assigning Left Sub tree? WHat about right sub tree? (Austin: This is for creating internal node, there is no left/right tree. See next 2 comments)
		{
			if(i < newInternal->size)
				newInternal->key[i] = virtualKey[j];
			newInternal->ptr[i] = virtualPtr[j];
		}
		if (cursor == root) // Need somewhere to Update the Height and Number of Nodes in B+ tree
		{
			Node *newRoot = new Node;
			newRoot->key[0] = cursor->key[cursor->size];
			//Austin: Point the internal node pointers to left and right sub tree accordingly
			newRoot->ptr[0] = cursor;
			newRoot->ptr[1] = newInternal;
			newRoot->IS_LEAF = false;
			newRoot->size = 1;
			root = newRoot;
			this->height++;
		}
		else // Update the parent Node's key
		{
			insertInternal(cursor->key[cursor->size], findParent(root, cursor), newInternal);
		}
	}
}
Node *BPTree::findParent(Node *cursor, Node *child)
{
	Node *parent = nullptr;
	if (cursor->IS_LEAF || (cursor->ptr[0])->IS_LEAF)
	{
		return nullptr;
	}
	for (int i = 0; i < cursor->size + 1; i++)
	{
		if (cursor->ptr[i] == child)
		{
			parent = cursor;
			return parent;
		}
		else
		{
			parent = findParent(cursor->ptr[i], child);
			if (parent != nullptr)
				return parent;
		}
	}
	return parent;
}
void BPTree::remove(int x)
{
	if (root == nullptr)
	{
		cout << "Tree empty\n";
	}
	else
	{
		Node *cursor = root;
		Node *parent = nullptr;
		int leftSibling, rightSibling;
		while (cursor->IS_LEAF == false)
		{
			for (int i = 0; i < cursor->size; i++)
			{
				parent = cursor;
				leftSibling = i - 1;
				rightSibling = i + 1;
				if (x < cursor->key[i])
				{
					cursor = cursor->ptr[i];
					break;
				}
				if (i == cursor->size - 1)
				{
					leftSibling = i;
					rightSibling = i + 2;
					cursor = cursor->ptr[i + 1];
					break;
				}
			}
		}
		bool found = false;
		int pos;
		for (pos = 0; pos < cursor->size; pos++)
		{
			if (cursor->key[pos] == x)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			cout << "Not found\n";
			return;
		}
		for (int i = pos; i < cursor->size; i++)
		{
			cursor->key[i] = cursor->key[i + 1];
		}
		cursor->size--;
		if (cursor == root)
		{
			for (int i = 0; i < MAX + 1; i++)
			{
				cursor->ptr[i] = nullptr;
			}
			if (cursor->size == 0)
			{
				cout << "Tree died\n";
				delete[] cursor->key;
				delete[] cursor->ptr;
				delete cursor;
				root = nullptr;
			}
			return;
		}
		cursor->ptr[cursor->size] = cursor->ptr[cursor->size + 1];
		cursor->ptr[cursor->size + 1] = nullptr;
		if (cursor->size >= (MAX + 1) / 2)
		{
			return;
		}
		if (leftSibling >= 0)
		{
			Node *leftNode = parent->ptr[leftSibling];
			if (leftNode->size >= (MAX + 1) / 2 + 1)
			{
				for (int i = cursor->size; i > 0; i--)
				{
					cursor->key[i] = cursor->key[i - 1];
				}
				cursor->size++;
				cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
				cursor->ptr[cursor->size - 1] = nullptr;
				cursor->key[0] = leftNode->key[leftNode->size - 1];
				leftNode->size--;
				leftNode->ptr[leftNode->size] = cursor;
				leftNode->ptr[leftNode->size + 1] = nullptr;
				parent->key[leftSibling] = cursor->key[0];
				return;
			}
		}
		if (rightSibling <= parent->size)
		{
			Node *rightNode = parent->ptr[rightSibling];
			if (rightNode->size >= (MAX + 1) / 2 + 1)
			{
				cursor->size++;
				cursor->ptr[cursor->size] = cursor->ptr[cursor->size - 1];
				cursor->ptr[cursor->size - 1] = nullptr;
				cursor->key[cursor->size - 1] = rightNode->key[0];
				rightNode->size--;
				rightNode->ptr[rightNode->size] = rightNode->ptr[rightNode->size + 1];
				rightNode->ptr[rightNode->size + 1] = nullptr;
				for (int i = 0; i < rightNode->size; i++)
				{
					rightNode->key[i] = rightNode->key[i + 1];
				}
				parent->key[rightSibling - 1] = rightNode->key[0];
				return;
			}
		}
		if (leftSibling >= 0)
		{
			Node *leftNode = parent->ptr[leftSibling];
			for (int i = leftNode->size, j = 0; j < cursor->size; i++, j++)
			{
				leftNode->key[i] = cursor->key[j];
			}
			leftNode->ptr[leftNode->size] = nullptr;
			leftNode->size += cursor->size;
			leftNode->ptr[leftNode->size] = cursor->ptr[cursor->size];
			removeInternal(parent->key[leftSibling], parent, cursor);
			delete[] cursor->key;
			delete[] cursor->ptr;
			delete cursor;
		}
		else if (rightSibling <= parent->size)
		{
			Node *rightNode = parent->ptr[rightSibling];
			for (int i = cursor->size, j = 0; j < rightNode->size; i++, j++)
			{
				cursor->key[i] = rightNode->key[j];
			}
			cursor->ptr[cursor->size] = nullptr;
			cursor->size += rightNode->size;
			cursor->ptr[cursor->size] = rightNode->ptr[rightNode->size];
			cout << "Merging two leaf nodes\n";
			removeInternal(parent->key[rightSibling - 1], parent, rightNode);
			delete[] rightNode->key;
			delete[] rightNode->ptr;
			delete rightNode;
		}
	}
}
void BPTree::removeInternal(int x, Node *cursor, Node *child)
{
	if (cursor == root)
	{
		if (cursor->size == 1)
		{
			if (cursor->ptr[1] == child)
			{
				delete[] child->key;
				delete[] child->ptr;
				delete child;
				root = cursor->ptr[0];
				delete[] cursor->key;
				delete[] cursor->ptr;
				delete cursor;
				cout << "Changed root node\n";
				return;
			}
			else if (cursor->ptr[0] == child)
			{
				delete[] child->key;
				delete[] child->ptr;
				delete child;
				root = cursor->ptr[1];
				delete[] cursor->key;
				delete[] cursor->ptr;
				delete cursor;
				cout << "Changed root node\n";
				return;
			}
		}
	}
	int pos;
	for (pos = 0; pos < cursor->size; pos++)
	{
		if (cursor->key[pos] == x)
		{
			break;
		}
	}
	for (int i = pos; i < cursor->size; i++)
	{
		cursor->key[i] = cursor->key[i + 1];
	}
	for (pos = 0; pos < cursor->size + 1; pos++)
	{
		if (cursor->ptr[pos] == child)
		{
			break;
		}
	}
	for (int i = pos; i < cursor->size + 1; i++)
	{
		cursor->ptr[i] = cursor->ptr[i + 1];
	}
	cursor->size--;
	if (cursor->size >= (MAX + 1) / 2 - 1)
	{
		return;
	}
	if (cursor == root)
		return;
	Node *parent = findParent(root, cursor);
	int leftSibling, rightSibling;
	for (pos = 0; pos < parent->size + 1; pos++)
	{
		if (parent->ptr[pos] == cursor)
		{
			leftSibling = pos - 1;
			rightSibling = pos + 1;
			break;
		}
	}
	if (leftSibling >= 0)
	{
		Node *leftNode = parent->ptr[leftSibling];
		if (leftNode->size >= (MAX + 1) / 2)
		{
			for (int i = cursor->size; i > 0; i--)
			{
				cursor->key[i] = cursor->key[i - 1];
			}
			cursor->key[0] = parent->key[leftSibling];
			parent->key[leftSibling] = leftNode->key[leftNode->size - 1];
			for (int i = cursor->size + 1; i > 0; i--)
			{
				cursor->ptr[i] = cursor->ptr[i - 1];
			}
			cursor->ptr[0] = leftNode->ptr[leftNode->size];
			cursor->size++;
			leftNode->size--;
			return;
		}
	}
	if (rightSibling <= parent->size)
	{
		Node *rightNode = parent->ptr[rightSibling];
		if (rightNode->size >= (MAX + 1) / 2)
		{
			cursor->key[cursor->size] = parent->key[pos];
			parent->key[pos] = rightNode->key[0];
			for (int i = 0; i < rightNode->size - 1; i++)
			{
				rightNode->key[i] = rightNode->key[i + 1];
			}
			cursor->ptr[cursor->size + 1] = rightNode->ptr[0];
			for (int i = 0; i < rightNode->size; ++i)
			{
				rightNode->ptr[i] = rightNode->ptr[i + 1];
			}
			cursor->size++;
			rightNode->size--;
			return;
		}
	}
	if (leftSibling >= 0)
	{
		Node *leftNode = parent->ptr[leftSibling];
		leftNode->key[leftNode->size] = parent->key[leftSibling];
		for (int i = leftNode->size + 1, j = 0; j < cursor->size; j++)
		{
			leftNode->key[i] = cursor->key[j];
		}
		for (int i = leftNode->size + 1, j = 0; j < cursor->size + 1; j++)
		{
			leftNode->ptr[i] = cursor->ptr[j];
			cursor->ptr[j] = nullptr;
		}
		leftNode->size += cursor->size + 1;
		cursor->size = 0;
		removeInternal(parent->key[leftSibling], parent, cursor);
	}
	else if (rightSibling <= parent->size)
	{
		Node *rightNode = parent->ptr[rightSibling];
		cursor->key[cursor->size] = parent->key[rightSibling - 1];
		for (int i = cursor->size + 1, j = 0; j < rightNode->size; j++)
		{
			cursor->key[i] = rightNode->key[j];
		}
		for (int i = cursor->size + 1, j = 0; j < rightNode->size + 1; j++)
		{
			cursor->ptr[i] = rightNode->ptr[j];
			rightNode->ptr[j] = nullptr;
		}
		cursor->size += rightNode->size + 1;
		rightNode->size = 0;
		removeInternal(parent->key[rightSibling - 1], parent, rightNode);
	}
}
void BPTree::display(Node *cursor)
{
	//Create a vector (array) of strings. 1 for each height of B+ tree
	vector<string> lines;
	for (int i = 0; i < this->height; i++)
		lines.push_back(""); 
	display(cursor, 0, &lines);

	//Best effort beautify print of each level
	for (int i = 0; i < this->height; i++) {
		for (int z = this->height - i - 1; z > 0; z--)
			cout << "            ";
		cout << lines[i] << endl;
	}
}

void BPTree::display(Node *cursor, int level, vector<string>* txtOutput)
{
	for (int i = 0; i < cursor->size; i++)
	{
		(*txtOutput)[level] = (*txtOutput)[level].append(to_string(cursor->key[i]) + " ");
	}
	(*txtOutput)[level] = (*txtOutput)[level].append("    ");

	if (cursor->IS_LEAF != true)
	{
		for (int i = 0; i < cursor->size + 1; i++)
		{
			display(cursor->ptr[i],level + 1,txtOutput);
			(*txtOutput)[level] = (*txtOutput)[level].append("    ");
		}
	}
}

Node *BPTree::getRoot()
{
	return root;
}

// Not sure why it is not working (Austin: Fixed)
int BPTree::count_nodes(Node *cursor) {
	int retVal = 1; //As long as we enter this method, count as 1 as we are iterating a node to be in this method (this is necessary to account for internal nodes).

	if (cursor == nullptr) //Edge case
		return 0;

	if (!cursor->IS_LEAF)
	{
		//If it is not a leaf node, we recurse through all available child nodes
		for (int i = 0; i < cursor->size + 1; i++)
			retVal += count_nodes(cursor->ptr[i]);
	}
	return retVal;
}
