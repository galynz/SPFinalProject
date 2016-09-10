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


//typedef enum { MAX_SPREAD, RANDOM, INCREMENTAL } SplitMethod;
//typedef enum {false, true} bool;

typedef struct sp_kd_tree {
    int dim;
    double val;
    SPKDTree left;
    SPKDTree right;
    SPPoint data;
}*SPKDTree;


SPKDTree init_tree(SPPoint* arr, int size, SplitMethod split_method, int prev_dim){
    SPKDArray kd_arr = NULL;
	SPKDArray* splitted_kd_arr = NULL;
    SPKDTree tree = NULL;
	int i = 0, dim = 0, split_dim = 0, max_dim = 0;
	double max_spread = 0, cur_spread = 0;
    
    if (size == INVALID || prev_dim == INVALID ){
        //TODO: add logger comment
        return NULL;
    }
    
    tree = (SPKDTree)malloc(sizeof(*tree));
    if (tree==NULL){
        //TODO: add logger comment
        return NULL;
    }
    
    if ( size == SINGLE_NODE ){
        //If len(array) = 1, create a node
        tree->dim = INVALID;
        tree->val = INVALID;
        tree->left = NULL;
        tree->right = NULL;
        tree->data = arr[0];
        return tree;
    }
    
    //Creating a kd array
    kd_arr = init_array(arr, size);
    dim = get_dim(kd_arr);
    
    //Decieding which dim to split according to
    if (split_method == MAX_SPREAD){
        //Finding the dim with the max spread
        max_dim = INVALID;
        max_spread = INVALID;
        for (i=0; i<=dim; i++){
            cur_spread = get_spread(kd_arr, i);
            if ( cur_spread > max_spread ){
                //If the current spread is greater than the max spread, 
                //update max dim to be the current dim
                max_spread = cur_spread;
                max_dim = i;
            }
        }
        split_dim = max_dim;
    } 
    else if (split_method == RANDOM ){
        //choosing a random dim
        split_dim = rand()/(RAND_MAX/(dim+1));
    }
    else if (split_method == INCREMENTAL) {
        //incremeting the previous dim
        split_dim = (prev_dim + 1)%dim;
    } else {
        destroy_kd_array(kd_arr, dim);
        free(tree);
        return NULL;
    }
    
    //Creating the tree
    //kd_arr = init(arr, size);
    splitted_kd_arr = split(kd_arr, split_dim);
    tree->dim = split_dim;
    tree->val = get_median(kd_arr, split_dim);
    tree->left = init_tree(get_arr(splitted_kd_arr[LEFT]), get_size(splitted_kd_arr[LEFT]), split_method, split_dim);
    tree->right = init_tree(get_arr(splitted_kd_arr[RIGHT]),get_size(splitted_kd_arr[RIGHT]), split_method, split_dim);
    tree->data = NULL;
    
    destroy_kd_array(kd_arr, dim);
    return tree;
}


bool isLeaf(SPKDTree node){
    //checking if a node is a leaf (it's value is INVALID)
    if (node->val == INVALID){
        return true;
    }
    return false;
}


void kNearestNeighbors(SPKDTree curr , SPBPQueue bpq, SPPoint p){
    //Initinlaizing vars
    SPListElement curr_elem = NULL;
    SP_BPQUEUE_MSG enqueue_msg;
    double curr_dim_distance = 0, curr_dim_distance_squared = 0, 
           queue_peek_last = 0;
    
    if (curr == NULL){
        return;
    }
    
    //If curr is a leaf, trying to add it to the sp
    if (isLeaf(curr) == true){
        curr_elem = spListElementCreate(spPointGetIndex(curr->data),
                        spPointL2SquaredDistance(p, curr->data));
        if (curr_elem == NULL){
            return;
        }
		enqueue_msg = spBPQueueEnqueue(bpq, curr_elem);
        return;
    }
    
    //Calculating the distance between curr and the point (in current dim)
    curr_dim_distance = curr->val - spPointGetAxisCoor(p, curr->dim);
    curr_dim_distance_squared = curr_dim_distance * curr_dim_distance;
    
    //Decieding which sub tree to search
    if (spPointGetAxisCoor(p, curr->dim) <= curr->val){
        //Searching the left sub tree
        kNearestNeighbors(curr->left, bpq, p);
        queue_peek_last = spListElementGetValue(spBPQueuePeekLast(bpq));
        if (spBPQueueIsFull(bpq) == false ||
            queue_peek_last > curr_dim_distance_squared){
            //Searching the right sub tree as well
            kNearestNeighbors(curr->right, bpq, p);
        }
    } else {
        //Searching the right sub tree
        kNearestNeighbors(curr->right, bpq, p);
        queue_peek_last = spListElementGetValue(spBPQueuePeekLast(bpq));
        if (spBPQueueIsFull(bpq) == false || 
            queue_peek_last > curr_dim_distance_squared){
            //Searching the left sub tree as well
            kNearestNeighbors(curr->left, bpq, p);
        }
    }
    
}

void destroy_tree(SPKDTree tree){
    if (tree == NULL) return;
    destroy_tree(tree->left);
    destroy_tree(tree->right);
    spPointDestroy(tree->data);
}