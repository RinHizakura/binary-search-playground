#ifndef B_TREE_H
#define B_TREE_H

#define B 16

#define KEY(btree, k, i) btree[k * B + i]

// Get the node index for the child i of node k
#define go(k, i) ((k) * (B + 1) + (i) + 1)

#endif
