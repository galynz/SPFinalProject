#include <stdio.h>
#include <stdlib.h>
#include "SPKDArray.h"
#include "SPPoint.h"
#include <string.h>

#define LEFT_INDEX 0
#define RIGHT_INDEX 1
#define MOVED_POINT_INDEX -1

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
}

SPKDArray init(SPPoint* arr, int size){
    SPPoint* arr_copy;
    SPKDArray kd_array;
    int i,j, k, index, dim;
	struct KDArrayDimCoor* dim_coor;
    
    //Copy the array
	arr_copy = (SPPoint*)malloc(sizeof(SPPoint)*size);
	//check if alloaction failed
	if (arr_copy == NULL) {
		return NULL;
	}
    memcpy(arr_copy, arr, sizeof(SPPoint*)*size); 

    
    dim = spPointGetDimension(arr_copy[0]);
	kd_array = (SPKDArray)malloc(sizeof(SPKDArray));
	if (kd_array == NULL) {
		free(arr_copy);
		return NULL;
	}
    kd_array->arr = arr_copy;
    kd_array->num = size;
    kd_array->dim = dim;
    
    //Allocating the matrix
    kd_array->matrix = (int**) malloc(dim*sizeof(int*));
    //If the allocation failed, free previous allocations
    if (kd_array->matrix == NULL){
        free(arr_copy);
        return NULL;
    }

	//creating an array of all the points' i-th coordiante
	dim_coor = (struct KDArrayDimCoor*)malloc(size * sizeof(struct KDArrayDimCoor));
	if (dim_coor == NULL) {
		free(kd_array);
		free(arr_copy);
		return NULL;
	}
    
    //Filling the matrix
    for (i=0; i<dim; i++){

        for (j=0; j<size; j++){
			/*
			dim_coor[j] = (struct KDArrayDimCoor)malloc(sizeof(struct KDArrayDimCoor));
			if (dim_coor[j] == NULL) {
				for (k = 0; k < j; k++) {
					free(dim_coor[k]);
				}
				free(dim_coor);
				free(kd_array);
				free(arr_copy);
			}
			*/
			dim_coor[j] = (struct KDArrayDimCoor){ spPointGetAxisCoor(arr_copy[j], i), j };
			/*
            dim_coor[j]->coor = spPointGetAxisCoor(arr_copy[j], i);
            dim_coor[j]->index = j;
			*/
        }
        //Sorting the array
        qsort(dim_coor, size, sizeof(struct KDArrayDimCoor), compareKDArrayDimCoor);
        
        //Allocating a row in the matrix
        kd_array->matrix[i] = (int*)malloc(sizeof(int)*size);
        if (kd_array->matrix[i] == NULL){
            //If allocation failed, free previous allocations
            for (j=0; j<=i; j++){
                free(kd_array->matrix[j]);
            }
            free(kd_array->matrix);
            free(arr_copy);
            return NULL;
        }
        
        //Filling the row
        for (j=0; j<size; j++){
            index = dim_coor[j].index;
            kd_array->matrix[i][index] = j;
        }
    }
	free(dim_coor);
    return kd_array;
}

SPKDArray* split(SPKDArray kd_arr, int coor){
    SPKDArray * ret_array;
    int median, i, j, left_cur_pos, right_cur_pos, left_index, right_index;

	int* sort_arr_left; 
	int* sort_arr_right; 
	int* map_left;
	int* map_right;

	sort_arr_left = (int*)malloc(kd_arr->num * sizeof(int));
	if (sort_arr_left == NULL) {
		return NULL;
	}
	sort_arr_right = (int*)malloc(kd_arr->num * sizeof(int));
	if (sort_arr_right == NULL) {
		free(sort_arr_left);
		return NULL;
	}
	map_left = (int*)malloc(kd_arr->num * sizeof(int));
	if (map_left == NULL) {
		free(sort_arr_left);
		free(sort_arr_right);
		return NULL;
	}
	map_right = (int*)malloc(kd_arr->num * sizeof(int));
	if (map_right == NULL) {
		free(sort_arr_left);
		free(sort_arr_right);
		free(map_left);
		return NULL;
	}
    
    ret_array = (SPKDArray*)malloc(2*sizeof(SPKDArray));
    if (ret_array == NULL){
		free(sort_arr_left);
		free(sort_arr_right);
		free(map_left);
		free(map_right);
        return NULL;
    }
    
    median = (kd_arr->num)/2;
	ret_array[LEFT_INDEX] = (SPKDArray)malloc(sizeof(SPKDArray));
	if (ret_array[LEFT_INDEX] == NULL) {
		free(sort_arr_left);
		free(sort_arr_right);
		free(map_left);
		free(map_right);
		free(ret_array);
	}
	ret_array[RIGHT_INDEX] = (SPKDArray)malloc(sizeof(SPKDArray));
	if (ret_array[RIGHT_INDEX] == NULL) {
		free(sort_arr_left);
		free(sort_arr_right);
		free(map_left);
		free(map_right);
		free(ret_array[LEFT_INDEX]);
		free(ret_array);
	}
    ret_array[LEFT_INDEX]->num = (kd_arr->num - median);
    ret_array[RIGHT_INDEX]->num = median;
    ret_array[LEFT_INDEX]->dim = kd_arr->dim;
    ret_array[RIGHT_INDEX]->dim = kd_arr->dim;
    
    
    //Creating the new arrays (left&right)
    for (i=0; i<=RIGHT_INDEX; i++){
        ret_array[i]->arr = (SPPoint*)malloc(sizeof(SPPoint*)*ret_array[i]->num);
        
        //If the allocation failed, free all previous allocations
        if (ret_array[i]->arr == NULL){
            for (j=0; j<i; j++){
                free(ret_array[j]->arr);
            }
			free(sort_arr_left);
			free(sort_arr_right);
			free(map_left);
			free(map_right);
			free(ret_array[LEFT_INDEX]);
			free(ret_array[RIGHT_INDEX]);
			free(ret_array);
            return NULL;
        }
    }
    
    //Dividing the points into left and right
    left_cur_pos = 0;
    right_cur_pos = median + 1;
    for (i=0; i<kd_arr->num; i++){
        if (kd_arr->matrix[coor][i] <= median){
            ret_array[LEFT_INDEX]->arr[left_cur_pos] = kd_arr->arr[i];
            //Updating mapping arrays
            map_left[i] = left_cur_pos; //mapping old_index:new_index
            map_right[i] = MOVED_POINT_INDEX;
            left_cur_pos++;
        } else {
            ret_array[RIGHT_INDEX]->arr[right_cur_pos] = kd_arr->arr[i];
            //Updating mapping arrays
            map_right[i] = right_cur_pos;
            map_left[i] = MOVED_POINT_INDEX;
            right_cur_pos++;
        }
    }
    //Allocating new matrixes
	ret_array[LEFT_INDEX]->matrix = (int**)malloc(ret_array[LEFT_INDEX]->dim*sizeof(int*));
	if (ret_array[LEFT_INDEX]->matrix == NULL) {
			for (j = LEFT_INDEX; j<=RIGHT_INDEX; j++) {
				free(ret_array[j]->arr);
			}
			free(sort_arr_left);
			free(sort_arr_right);
			free(map_left);
			free(map_right);
			free(ret_array[LEFT_INDEX]);
			free(ret_array[RIGHT_INDEX]);
			free(ret_array);
	}

	ret_array[RIGHT_INDEX]->matrix = (int**)malloc(ret_array[RIGHT_INDEX]->dim * sizeof(int*));
	if (ret_array[RIGHT_INDEX]->matrix == NULL) {
		for (j = LEFT_INDEX; j <= RIGHT_INDEX; j++) {
			free(ret_array[j]->arr);
		}
		free(sort_arr_left);
		free(sort_arr_right);
		free(map_left);
		free(map_right);
		free(ret_array[LEFT_INDEX]->matrix);
		free(ret_array[RIGHT_INDEX]);
		free(ret_array[LEFT_INDEX]);
		free(ret_array);
	}

    
    for (i=0; i<kd_arr->dim; i++){
		ret_array[LEFT_INDEX]->matrix[i] = (int*)malloc(ret_array[LEFT_INDEX]->num * sizeof(int));
		if (ret_array[LEFT_INDEX]->matrix[i] == NULL) {
			for (j = LEFT_INDEX; j <= RIGHT_INDEX; j++) {
				free(ret_array[j]->arr);
			}
			free(sort_arr_left);
			free(sort_arr_right);
			free(map_left);
			free(map_right);
			free(ret_array[LEFT_INDEX]->matrix);
			free(ret_array[RIGHT_INDEX]->matrix);
			free(ret_array[RIGHT_INDEX]);
			free(ret_array[LEFT_INDEX]);
			free(ret_array);
		}
        for (j=0; j<kd_arr->num; j++){
            if (map_left[j] > MOVED_POINT_INDEX){
                //Saving the indexes of the points in the left array according to their order in the i-th dim
                sort_arr_left[kd_arr->matrix[i][j]] = map_left[j];
                //Putting -1 in the right array (to mark that the point doens't belong here)
                sort_arr_right[kd_arr->matrix[i][j]] = MOVED_POINT_INDEX;
            } else {
                //Saving the indexes of the points in the right array according to their order in the i-th dim
                //sort_arr_right[kd_arr->matrix[i][map_right[j]]] = j;
				sort_arr_right[kd_arr->matrix[i][j]] = map_right[j];
                //Putting -1 in the left array (to mark that the point doens't belong here)
                //sort_arr_left[kd_arr->matrix[i][map_right[j]]] = MOVED_POINT_INDEX;
				sort_arr_left[kd_arr->matrix[i][j]] = MOVED_POINT_INDEX;
            }
        }
        
        //Updating left and right matrices
        left_cur_pos = 0;
        right_cur_pos = 0;
        for (j=0; j<kd_arr->num; j++){
            //The first index that isn't -1 (in left/right sort_arr) will be in left_cur_pos/right_cur_pos
            if (sort_arr_left[j] > MOVED_POINT_INDEX){
                left_index = map_left[j]; //getting the j-th point's new index
                ret_array[LEFT_INDEX]->matrix[i][left_index] = left_cur_pos;
                left_cur_pos++;
            } else {
                right_index = map_right[j]; //getting the j-th point's new index
                ret_array[RIGHT_INDEX]->matrix[i][right_index] = right_cur_pos;
                right_cur_pos++;
            }
        }
    }
    
    return ret_array;
    
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
	int median_index;
	double median_value;
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