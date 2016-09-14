#ifndef SPKDTREE_H_
#define SPKDTREE_H_

#include "SPBPriorityQueue.h"

typedef enum { MAX_SPREAD, RANDOM, INCREMENTAL } SplitMethod;

typedef struct sp_kd_tree *SPKDTree;

/**
 * Creates a new kd-tree struct by creating a kd-array
 * and spliting it to left and right kd-arrays according to split_method.
 * Uses a recursive helper function so that the kd-array won't be created all over again each iteration.
 * 
 * @param arr - an array of SPPoints, all with the same number of coordinates.
 * @param size - the number of SPPoints in arr.
 * @param split_method - SplitMethod (max_spread/random/incremental)
 * @return NULL if size <= 0 or dim <= 0
 * or not all the points have the same dim
 * or memory allocation failed. Otherwise return SPKDTree.
 */

SPKDTree initTree(SPPoint* arr, int size, SplitMethod split_method);

/**
 *  A recursive function that enqueues the nearest neighbors of p to bpq.
 *
 * @param curr - a SPKDTree to start the search from.
 * @param bpq - a SPBPQueue to store the neighbors in.
 * @param p - a SPPoint to search.
 */

void kNearestNeighbors(SPKDTree curr , SPBPQueue bpq, SPPoint p);

/**
 * A recursive function that frees all the memory allocated when the tree was created.
 */

void destroyTree(SPKDTree tree);

#endif /* SPKDTREE_H_ */