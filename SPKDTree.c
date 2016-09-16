#include <stdio.h>
#include <stdlib.h>
#include "SPKDArray.h"
#include "SPKDTree.h"
#include "SPPoint.h"
#include "SPBPriorityQueue.h"
#include "SPListElement.h"


#define SINGLE_NODE 1
#define INVALID -1
#define LEFT 0
#define RIGHT 1
#define INIT_PREV_DIM -1
#define EMPTY_ARR 0

struct sp_kd_tree {
    int dim;
    double val;
    SPKDTree left;
    SPKDTree right;
    SPPoint data;
};

/**
A helper function that finds the max spread out of all the dimension.
*/
int getMaxSpreadDim(SPKDArray kd_arr, int dim) {
	int max_dim = 0;
	double cur_spread = 0, max_spread = 0;
	int i = 0;
	//Finding the dim with the max spread
	for (i = 0; i<dim; i++) {
		cur_spread = getSpread(kd_arr, i);
		if (cur_spread > max_spread) {
			//If the current spread is greater than the max spread, 
			//update max dim to be the current dim
			max_spread = cur_spread;
			max_dim = i;
		}
	}
	return max_dim;
}

SPKDTree initTreeRec(SPPoint* arr, int size, SplitMethod split_method, int prev_dim, SPKDArray kd_arr){
	SPKDArray* splitted_kd_arr = NULL;
    SPKDTree tree = NULL;
	int dim = 0, split_dim = 0;
    
    tree = (SPKDTree)malloc(sizeof(*tree));
    if (tree==NULL){
        //TODO: add logger comment
        return NULL;
    }
    
    //If size(array) = 1, creates a node
    if ( size == SINGLE_NODE ){
        tree->dim = INVALID;
        tree->val = INVALID;
        tree->left = NULL;
        tree->right = NULL;
        tree->data = arr[0];
        return tree;
    }
    
    dim = getDim(kd_arr);
    
    //Decieding which dim to split according to
    if (split_method == MAX_SPREAD){
        split_dim = getMaxSpreadDim(kd_arr, dim);
    } 
    else if (split_method == RANDOM ){
        //choosing a random dim
        split_dim = rand()/(RAND_MAX/dim);
    }
    else if (split_method == INCREMENTAL) {
        //incremeting the previous dim
        split_dim = (prev_dim + 1)%dim;
    } else {
        destroyKdArray(kd_arr, dim);
        free(tree);
        return NULL;
    }
    
    //Creating the tree
    //kd_arr = init(arr, size);
    splitted_kd_arr = split(kd_arr, split_dim);
    tree->dim = split_dim;
    tree->val = getMedian(kd_arr, split_dim);
    tree->left = initTreeRec(getArr(splitted_kd_arr[LEFT]), getSize(splitted_kd_arr[LEFT]), split_method, split_dim, splitted_kd_arr[LEFT]);
    tree->right = initTreeRec(getArr(splitted_kd_arr[RIGHT]),getSize(splitted_kd_arr[RIGHT]), split_method, split_dim, splitted_kd_arr[RIGHT]);
    tree->data = NULL;
    
    //Destroying the kd-arrays created in the current iteration
    destroyKdArray(splitted_kd_arr[LEFT], dim);
    destroyKdArray(splitted_kd_arr[RIGHT], dim);
    return tree;
}

SPKDTree initTree(SPPoint* arr, int size, SplitMethod split_method) {
	SPKDArray kd_arr = NULL;
	SPKDTree tree = NULL;
	int dim = 0;

	if (size <= EMPTY_ARR) {
		//TODO: add logger comment
		return NULL;
	}

	//Creating a kd array
	kd_arr = initArray(arr, size);
	if (kd_arr == NULL) {
		return NULL;
	}

	//Creating the kd tree
	tree = initTreeRec(arr, size, split_method, INIT_PREV_DIM, kd_arr);
	if (tree == NULL) {
		destroyKdArray(kd_arr, dim);
		return NULL;
	}

	//Destroying the kd array
	dim = getDim(kd_arr);
	destroyKdArray(kd_arr, dim);

	return tree;
}

bool isLeaf(SPKDTree node){
    //checking if a node is a leaf (it's value is INVALID)
    if (node->val == INVALID){
        return true;
    }
    return false;
}

/**
 A helper function that searches one of the tree's sides (or both)
 */

status searchSubTree(SPKDTree search_sub, SPKDTree other, SPBPQueue bpq, SPPoint p, double curr_dim_distance_squared){
    status ret_status = SUCCESS;
    double queue_peek_last = 0;
    //Searching the sub tree
    ret_status = kNearestNeighbors(search_sub, bpq, p);
    if ( ret_status == FAILURE ){  
        return FAILURE;
    }
    queue_peek_last = spListElementGetValue(spBPQueuePeekLast(bpq));
    if ( queue_peek_last == INVALID ){
        return FAILURE;
    }
    if (spBPQueueIsFull(bpq) == false || 
        queue_peek_last > curr_dim_distance_squared){
        //Searching the other sub tree as well
        ret_status = kNearestNeighbors(other, bpq, p);
        if (ret_status == FAILURE ){
            return FAILURE;
        }
    }
    return SUCCESS;
}

status kNearestNeighbors(SPKDTree curr , SPBPQueue bpq, SPPoint p){
    //Initinlaizing vars
    SPListElement curr_elem = NULL;
    SP_BPQUEUE_MSG enqueue_msg;
    double curr_dim_distance = 0, curr_dim_distance_squared = 0;
    status ret_status;
    
    if (curr == NULL){
        return FAILURE;
    }
    
    //If curr is a leaf, trying to add it to the sp
    if (isLeaf(curr) == true){
        curr_elem = spListElementCreate(spPointGetIndex(curr->data),
                        spPointL2SquaredDistance(p, curr->data));
        if (curr_elem == NULL){
            return FAILURE;
        }
		enqueue_msg = spBPQueueEnqueue(bpq, curr_elem);
        if (enqueue_msg != SP_BPQUEUE_SUCCESS){
            return FAILURE;
        }
        return SUCCESS;
    }
    
    //Calculating the distance between curr and the point (in current dim)
    curr_dim_distance = curr->val - spPointGetAxisCoor(p, curr->dim);
    curr_dim_distance_squared = curr_dim_distance * curr_dim_distance;
    
    //Decieding which sub tree to search
    if (spPointGetAxisCoor(p, curr->dim) <= curr->val){
        //Searching the left sub tree
        ret_status = searchSubTree(curr->left, curr->right, bpq, p, curr_dim_distance_squared);
    } else {
        //Searching the right sub tree
        ret_status = searchSubTree(curr->right, curr->left, bpq, p, curr_dim_distance_squared);
    }
    
    if (ret_status == FAILURE){
        return FAILURE;
    }
    return SUCCESS;
}

void destroyTree(SPKDTree tree){
    if (tree == NULL) return;
    //Recursivly destroy left and right sub trees
    destroyTree(tree->left);
    destroyTree(tree->right);
    spPointDestroy(tree->data);
    free(tree);
}