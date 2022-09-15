#include "BTreedelete.h"

int main() {
  BPTree node;
  node.insert(5);
  node.insert(15);
  node.insert(25);
  node.insert(35);
  node.insert(45);

  node.display(node.getRoot());
  node.search(15);
  node.remove(15);

  node.display(node.getRoot());

  node.search(15);
  node.rangequery(5,35);
}