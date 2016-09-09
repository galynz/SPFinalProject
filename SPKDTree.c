#include <stdio.h>
#include <stdlib.h>
#include "SPKDArray.h"

#define SINGLE_NODE 1
#define INVALID -1
#define LEFT 0
#define RIGHT 1


typedef enum { MAX_SPREAD, RANDOM, INCREMENTAL } SplitMethod;
typedef enum {false, true} bool;

typedef struct SPKDTree {
    int dim;
    int val;
    SPKDTree* left;
    SPKDTree* right;
    SPPoint* data;
}*SPKDTree;


SPKDTree init_tree(SPPoint* arr, int size, SplitMethod split_method, int prev_dim){
    SPKDArray kd_arr;
    SPKDTree tree;
    int i, i_spread, dim, split_dim, max_dim, max_spread, cur_spread;
    
    if (size == INVALID || dim == INVALID ){
        //TODO: add logger comment
        return NULL;
    }
    
    tree = (SPKDTree)malloc(sizeof(SPKDTree));
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
    
    kd_arr = init(arr, size);
    dim = kd_arr->dim;
    
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
        free(tree);
        return NULL;
    }
    
    //Creating the tree
    kd_arr = init(arr, size);
    splitted_kd_arr = split(kd_arr, split_dim);
    tree->dim = split_dim;
    tree->val = get_median(kd_arr, split_dim);
    tree->left = init_tree(splitted_kd_arr[LEFT], size, split_method, split_dim);
    tree->right = init_tree(splitted_kd_arr[RIGHT], size, split_method, split_dim);
    tree->data = NULL;
    return tree;
}


bool isLeaf(SPKDTree node){
    //checking if a node is a leaf (it's value is INVALID)
    if (node->val == INVALID){
        return true;
    }
    return false;
}