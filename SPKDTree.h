#ifndef SPKDTREE_H_
#define SPKDTREE_H_

#include "SPBPriorityQueue.h"

typedef enum { MAX_SPREAD, RANDOM, INCREMENTAL } SplitMethod;

typedef struct sp_kd_tree *SPKDTree;


SPKDTree init_tree(SPPoint* arr, int size, SplitMethod split_method, int prev_dim);

void kNearestNeighbors(SPKDTree curr , SPBPQueue bpq, SPPoint p);

void destroy_tree(SPKDTree tree);

#endif SPKDTREE_H_