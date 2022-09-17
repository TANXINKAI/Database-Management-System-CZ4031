#include "BTreeModel.h"

int main() {
  BPTree node;
  node.insert(5);
  node.insert(15);
  node.insert(25);
  node.insert(35);
  node.insert(45);

  node.display(node.getRoot());

  // Trying to print Number of Nodes here but its not working, in BTreeModel.h line 642
  int counts;
  counts = node.count_nodes(node.getRoot(),0);
  cout << "Number of Nodes" << counts << "\n";

  node.search(15);
  node.remove(15);

  node.display(node.getRoot());

  node.search(15);
  node.rangequery(5,35);
}
