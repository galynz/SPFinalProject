#include <stdio.h>
#include <stdlib.h>
#include "SPKDArray.h"
#include "SPPoint.h"
#include <string.h>

#define NOT_IN_SUB_INDEX -1

enum SIDE {left, right};

struct sp_kd_array {
    int num;
    int dim;
    SPPoint* arr;
    int** matrix;
};

struct KDArrayDimCoor {
    double coor;
    int index;
};

int compareKDArrayDimCoor(const void * a, const void * b) {
	const struct KDArrayDimCoor* a_point_coor = (struct KDArrayDimCoor*)a;
	const struct KDArrayDimCoor* b_point_coor = (struct KDArrayDimCoor*)b;
	return (a_point_coor->coor > b_point_coor->coor) - (a_point_coor->coor < b_point_coor->coor);
    
    
SPKDArray init_array(SPPoint* arr, int size){   
    SPKDArray kd_array = NULL;
    int dim = 0, i = 0;
    
    //Creating the kd array
    dim = spPointGetDimension(arr[0]);
    kd_array = create_kd_array(arr, size, dim);
    
    //Filling the matrix
    for (i=0; i<dim; i++){
        kd_array[i] = sort_points_dim(kd_array->arr, i, size);
        if (kd_array[i] == NULL){
            destroy_kd_array(kd_array, i-1);
            return NULL;
        }
    }
    
    return kd_array;
}

static SPKDArray create_kd_array(SPPoint* arr, int size, int dim){
    SPKDArray kd_array = NULL;
    int i = 0, j = 0;
    
    //Allocating kd_array
    kd_array = (SPKDArray)malloc(sizeof(SPKDArray));
    if (kd_array == NULL) {
        return NULL;
    }
    
    kd_array->num = size;
    kd_array->dim = dim;
    
    //Copy the array
	kd_array->arr = (SPPoint*)malloc(sizeof(SPPoint)*size);
	//check if alloaction failed
	if (arr_copy == NULL) {
        free(kd_array);
		return NULL;
	}
    memcpy(kd_array->arr, arr, sizeof(SPPoint*)*size);
    
    //Allocating the matrix
    kd_array->matrix = (int**) malloc(dim*sizeof(int*));
    if (kd_array->matrix == NULL){
        free(kd_array->arr);
        free(kd_array);
        return NULL;
    }
    
    //Allocating each of the rows
    for (i=0; i<dim; i++){
        kd_array->matrix[i] = (int*)malloc(size*sizeof(int));
        if (kd_array->matrix[i] == NULL){
            for (j=0; j<i; j++){
                free(kd_array->matrix[j]);
            }
            free(kd_array->matrix);
            free(kd_array->arr);
            free(kd_array);
        }
    }
    
    return kd_array;
}

void destroy_kd_array(SPKDArray kd_array, int dim){
    int i = 0;
    
    //Free all allocations
    for (i=0; i<=dim; i++){
        free(kd_array->matrix[i]);
    }
    free(kd_array->matrix);
    free(kd_array->arr);
    free(kd_array);
}

static int* sort_points_dim(SPPoint* arr, int coor, int size){
    int* index_arr = NULL;
    struct KDArrayDimCoor* dim_coor = NULL;
    int i = 0, point_index = 0;
    
    //creating an array of all the points' i-th coordiante
	dim_coor = (struct KDArrayDimCoor*)malloc(size * sizeof(struct KDArrayDimCoor));
    if (dim_coor == NULL){
        return NULL;
    }
    for (i=0; i<size; i++){
        dim_coor[i] = (struct KDArrayDimCoor){ spPointGetAxisCoor(arr[i], coor), i };
    }  
    
    //Sorting the array
    qsort(dim_coor, size, sizeof(struct KDArrayDimCoor), compareKDArrayDimCoor);
    
    /*Filling the index array with the indexes of the points 
    according to their order in this coordiante*/
    for (i=0; i<size; i++){
        point_index = ret_arr[i].index;
        index_arr[point_index] = i;
    }
    
    //Free all allocations
    free(dim_coor);
    
    return index_arr;
}

SPKDArray* split(SPKDArray kd_arr, int coor){
    int median=0, size=0, i=0;
    SPKDArray * left_right_array = NULL;
    bool *left_indexes = NULL, *right_indexes = NULL;
    
    //Calculating the median
    size = kd_arr->num;
    median = size/2;
    
    //Allocating indexes' arrays
    left_indexes = (int*)malloc(median*sizeof(int));
    if (left_indexes  == NULL){
        return NULL;
    }
    right_indexes = (int*)malloc((size-median)*sizeof(int));
    if (right_indexes == NULL){
        free(left_indexes);
        return NULL;
    }
    
    //Filling the indexes' arrays
    for (i=0; i<size; i++){
        if (kd_arr->matrix[coor][i] <= median){
            left_indexes[i] = true;
            right_indexes[i] = false;
        } else {
            right_indexes[i] = true;
            left_indexes[i] = false;
        }
    }
    
    //Allocating left and right kd arrays
    left_right_array = (SPKDArray*)malloc(2*sizeof(SPKDArray));
    if (left_right_array == NULL){
        free(left_indexes);
        free(right_indexes);
        return NULL;
    }
    
    //Creating left and right kd arrays using the indexes
    left_right_array[left] = create_kd_array_sub_array(kd_arr, left_indexes, median);
    if (left_right_array[left] == NULL){
        free(left_indexes);
        free(right_indexes);
        free(left_right_array);
        return NULL;
    }
    left_right_array[right] = create_kd_array_sub_array(kd_arr, right_indexes, size-median);
    if (left_right_array[left] == NULL){
        free(left_indexes);
        free(right_indexes);
        destroy_kd_array(left_right_array[left], left_right_array[left]->dim);
        free(left_right_array);
        return NULL;
    }
     
    //Free allocations
    free(left_indexes);
    free(right_indexes);
    
    return left_right_array;
}

static SPKDArray create_kd_array_sub_array(SPKDArray kd_arr, bool* index_arr, int sub_size){
    //Initilazing vars
    int i=0, all_size=0, dim=0, old_index=0,
        sub_pos=0, j=0, curr_pos = 0,
        point_index = 0;
    SPKDArray sub_kd_array =  NULL;
    SPPoint* sub_arr = NULL;
    int* coor_sorting_arr = NULL;
    
    all_size = kd_arr->num;
    dim = kd_arr->dim;

    //Creating sub array of points
    sub_arr = (SPPoint*)malloc(sub_size*sizeof(SPPoint));
    if (sub_arr == NULL){
        return;
    }
    for (i=0; i<sub_size; i++){
        if (index_arr[i] == true){
            sub_arr[i] = kd_arr->arr[i];
        }
    }
    
    //Creating sub kd array
    sub_kd_array = create_kd_array(sub_arr, sub_size, dim);
    free(sub_arr);
    if ( sub_kd_array == NULL){
        return NULL;
    }
    
    /*
    Creating an array that will store each of the rows (of the matrix)
    according to their order in each coordiante, 
    without coordiantes of points that aren't in the sub array
    */
    coor_sorting_arr = (int*)malloc(all_size*sizeof(int));
    if (coor_sorting_arr == NULL){
        destroy_kd_array(sub_kd_array, 0);
        return NULL;
    }
    for (i=0; i<dim; i++){
        //Store the indexes of the points according to their i-th dim order
        for (j=0; j<all_size; j++){
            old_index = kd_arr->matrix[i][j];
            if (index_arr[j] == true){
                //If the point is in the sub array, put it's index in her old position in the coor array
                coor_sorting_arr[old_index] = j;
            } else {
                //Mark that the point isn't in the sub array
                coor_sorting_arr[old_index] = NOT_IN_SUB_INDEX;
            }
        }
        //Allocating the i-th row in sub matrix
        sub_kd_array->matrix[i] = (int*)malloc(sub_size*sizeof(int));
        if ( sub_kd_array->matrix[i] == NULL ){
            destroy_kd_array(sub_kd_array, i-1);
            free(coor_sorting_arr);
            return NULL;
        }
        //Re-index the i-th row of the matrix
        sub_pos = 0;
        for (i=0; i<all_size; i++){
            point_index = coor_sorting_arr[i];
            if ( point_index != NOT_IN_SUB_INDEX ){
                sub_kd_array->matrix[i][point_index] = sub_pos;
                sub_pos++;
            }
        }
    }
    
    //Return the sub kd array
    free(coor_sorting_arr);
    return sub_kd_array;
}

double get_spread(SPKDArray kd_arr, int dim){
	int min_index, max_index;
	double max_value, min_value;
    min_index = kd_arr->matrix[dim][0];
    max_index = kd_arr->matrix[dim][kd_arr->num-1];
    min_value = spPointGetAxisCoor(kd_arr->arr[min_index], dim);
    max_value = spPointGetAxisCoor(kd_arr->arr[max_index], dim);
    return (max_value - min_value);
}

double get_median(SPKDArray kd_arr, int dim){
	int median_index = 0;
	double median_value = 0;
    median_index = (kd_arr->num)/2;
	median_value = spPointGetAxisCoor(kd_arr->arr[median_index], dim);
    return median_value;
}

int get_dim(SPKDArray arr){
	return arr->dim;
}

SPPoint* get_arr(SPKDArray kd_arr) {
	return kd_arr->arr;
}