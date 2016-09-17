#include <stdio.h>
#include <stdlib.h>
#include "SPKDArray.h"
#include "SPPoint.h"
#include <string.h>
#include <stdbool.h>

#define NOT_IN_SUB_INDEX -1
#define ODD 1

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

static int compareKDArrayDimCoor(const void * a, const void * b) {
	const struct KDArrayDimCoor* a_point_coor = (struct KDArrayDimCoor*)a;
	const struct KDArrayDimCoor* b_point_coor = (struct KDArrayDimCoor*)b;
	return (a_point_coor->coor > b_point_coor->coor) - (a_point_coor->coor < b_point_coor->coor);
}

/**
  A helper function that allocates all the memory required when creating a kd array.
 */
static SPKDArray createKdArray(SPPoint* arr, int size, int dim) {
	SPKDArray kd_array = NULL;
	int i = 0, j = 0;

	//Allocating kd_array
	kd_array = (SPKDArray)malloc(sizeof(*kd_array));
	if (kd_array == NULL) {
		return NULL;
	}

	kd_array->num = size;
	kd_array->dim = dim;

	//Copy the array
	kd_array->arr = (SPPoint*)malloc(sizeof(SPPoint)*size);
	//check if alloaction failed
	if (kd_array->arr == NULL) {
		free(kd_array);
		return NULL;
	}
	memcpy(kd_array->arr, arr, sizeof(SPPoint)*size);

	//Allocating the matrix
	kd_array->matrix = (int**)malloc(dim * sizeof(int*));
	if (kd_array->matrix == NULL) {
		free(kd_array->arr);
		free(kd_array);
		return NULL;
	}

	return kd_array;
}

static SPKDArray createKdArraySubArray(SPKDArray kd_arr, bool* index_arr, int sub_size) {
	//Initilazing vars
	int i = 0, all_size = 0, dim = 0, old_index = 0,
		sub_pos = 0, j = 0;
	SPKDArray sub_kd_array = NULL;
	SPPoint* sub_arr = NULL;
	int* coor_sorting_arr = NULL, *map_index_arr = NULL;

	all_size = kd_arr->num;
	dim = kd_arr->dim;

	//Creating sub array of points
	sub_pos = 0;
	sub_arr = (SPPoint*)malloc(sub_size * sizeof(SPPoint));
	if (sub_arr == NULL) {
		return NULL;
	}
    map_index_arr = (int*)malloc(all_size * sizeof(int));
    if (map_index_arr == NULL){
        free(sub_arr);
        return NULL;
    }
	for (i = 0; i<all_size; i++) {
		if (index_arr[i] == true) {
			sub_arr[sub_pos] = kd_arr->arr[i];
            map_index_arr[i] = sub_pos;
			sub_pos++;
		} else {
            map_index_arr[i] = NOT_IN_SUB_INDEX;
        }
	}

	//Creating sub kd array
	sub_kd_array = createKdArray(sub_arr, sub_size, dim);
    free(sub_arr);
	if (sub_kd_array == NULL) {
        free(map_index_arr);
		return NULL;
	}
    
    for (i=0; i<dim; i++){
        //Allocating the i-th row in sub matrix
		sub_kd_array->matrix[i] = (int*)malloc(sub_size * sizeof(int));
		if (sub_kd_array->matrix[i] == NULL) {
			destroyKdArray(sub_kd_array, i);
			free(coor_sorting_arr);
			return NULL;
		}
		//Re-index the i-th row of the matrix
        sub_pos = 0;
        for (j = 0; j < all_size; j++){
            old_index = kd_arr->matrix[i][j];
            if (map_index_arr[old_index] != NOT_IN_SUB_INDEX){
                sub_kd_array->matrix[i][sub_pos] = map_index_arr[old_index];
                sub_pos++;
            }
        }
    }

	//Return the sub kd array
	free(map_index_arr);
	return sub_kd_array;
}

static int* sortPointsDim(SPPoint* arr, int coor, int size) {
	int* index_arr = NULL;
	struct KDArrayDimCoor* dim_coor = NULL;
	int i = 0, point_index = 0;

	//creating an array of all the points' i-th coordiante
	dim_coor = (struct KDArrayDimCoor*)malloc(size * sizeof(struct KDArrayDimCoor));
	if (dim_coor == NULL) {
		return NULL;
	}
	for (i = 0; i<size; i++) {
		dim_coor[i] = (struct KDArrayDimCoor) { spPointGetAxisCoor(arr[i], coor), i };
	}

	//Sorting the array
	qsort(dim_coor, size, sizeof(struct KDArrayDimCoor), compareKDArrayDimCoor);

	/*Filling the index array with the indexes of the points
	according to their order in this coordiante*/
	index_arr = (int*)malloc(size * sizeof(int));
	if (index_arr == NULL) {
		free(dim_coor);
		return NULL;
	}

	for (i = 0; i<size; i++) {
		point_index = dim_coor[i].index;
		index_arr[i] = point_index;
	}

	//Free all allocations
	free(dim_coor);

	return index_arr;
}
    
SPKDArray initArray(SPPoint* arr, int size){   
    SPKDArray kd_array = NULL;
    int dim = 0, i = 0;
    
    //Validating the array
    if (size <= 0){
        return NULL;
    }
    dim = spPointGetDimension(arr[0]);
    if (dim <= 0){
        return NULL;
    }
    
    //Making sure all the points have the same dim 
    for (i=0; i<size; i++){
        if ( spPointGetDimension(arr[i]) != dim ){
            return NULL;
        }
    }
    //Creating the kd array
    kd_array = createKdArray(arr, size, dim);
    if (kd_array == NULL){
        return NULL;
    }
    
    //Filling the matrix
    for (i=0; i<dim; i++){
        kd_array->matrix[i] = sortPointsDim(kd_array->arr, i, size);
        if (kd_array->matrix[i] == NULL){
            destroyKdArray(kd_array, i-1);
            return NULL;
        }
    }
    
    return kd_array;
}



void destroyKdArray(SPKDArray kd_array, int dim){
    int i = 0;
    
    //Free all allocations
    for (i=0; i<dim; i++){
        free(kd_array->matrix[i]);
    }
    free(kd_array->matrix);
    free(kd_array->arr);
    free(kd_array);
}



SPKDArray* split(SPKDArray kd_arr, int coor){
	int median = 0, size = 0, i = 0, left_size = 0, right_size=0, point_index;
    SPKDArray * left_right_array = NULL;
    bool *left_indexes = NULL, *right_indexes = NULL;
    
    //Calculating the median
    size = kd_arr->num;
    median = size/2;
	if (size % 2 == ODD) {
		median++;
	}
	left_size = median;
	right_size = size - left_size;
    
    //Allocating indexes' arrays
    left_indexes = (bool*)malloc(size*sizeof(bool));
    if (left_indexes  == NULL){
        return NULL;
    }
    right_indexes = (bool*)malloc(size*sizeof(bool));
    if (right_indexes == NULL){
        free(left_indexes);
        return NULL;
    }
    
    //Filling the indexes' arrays   
    for (i=0; i<size; i++){
        point_index = kd_arr->matrix[coor][i];
        if (i<median){
            left_indexes[point_index] = true;
            right_indexes[point_index] = false;
        } else {
            left_indexes[point_index] = false;
            right_indexes[point_index] = true;
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
    left_right_array[left] = createKdArraySubArray(kd_arr, left_indexes, left_size);
    if (left_right_array[left] == NULL){
        free(left_indexes);
        free(right_indexes);
        free(left_right_array);
        return NULL;
    }

    left_right_array[right] = createKdArraySubArray(kd_arr, right_indexes, right_size);
    if (left_right_array[right] == NULL){
        free(left_indexes);
        free(right_indexes);
        destroyKdArray(left_right_array[left], left_right_array[left]->dim);
        free(left_right_array);
        return NULL;
    }
     
    //Free allocations
    free(left_indexes);
    free(right_indexes);
    
    return left_right_array;
}



double getSpread(SPKDArray kd_arr, int dim){
	int min_index, max_index;
	double max_value, min_value;
    min_index = kd_arr->matrix[dim][0];
    max_index = kd_arr->matrix[dim][kd_arr->num-1];
    min_value = spPointGetAxisCoor(kd_arr->arr[min_index], dim);
    max_value = spPointGetAxisCoor(kd_arr->arr[max_index], dim);
    return (max_value - min_value);
}

double getMedian(SPKDArray kd_arr, int dim){
	int median_index = 0, point_index = 0;
	double median_value = 0;
	median_index = kd_arr->num / 2;
	if (kd_arr->num % 2 != ODD) {
		median_index--;
	}
	point_index = kd_arr->matrix[dim][median_index];
	median_value = spPointGetAxisCoor(kd_arr->arr[point_index], dim);
	
    return median_value;
}

int getDim(SPKDArray arr){
	return arr->dim;
}

SPPoint* getArr(SPKDArray kd_arr) {
	return kd_arr->arr;
}

int getSize(SPKDArray kd_arr) {
	return kd_arr->num;
}