#include <stdio.h>
#include <stdlib.h>

typedef struct SPKDArray {
    int num;
    int dim;
    SPPoint* arr;
    int** matrix;
}*SPKDArray;

struct KDArrayDimCoor {
    double coor;
    int index;
}

#define LEFT_INDEX 0
#define RIGHT_INDEX 1

SPKDArray init(SPPoint* arr, int size){
    SPPoint* arr_copy;
    SPKDArray kd_array;
    int i,j, index, dim;
    struct KDArrayDimCoor dim_coor[size];
    
    //Copy the array
    memccpy(arr_copy, arr, sizeof(SPPoint*)*size); 
    //check if alloaction failed
    if ( arr_copy == NULL ){
        return NULL;
    }
    
    dim = spPointGetDimension(arr_copy[0]);
    kd_array->arr = arr_copy;
    kd_array->num = size;
    kd_array->dim = dim;
    
    //Allocating the matrix
    kd_array.matrix = (int**) malloc(dim*sizeof(int*));
    //If the allocation failed, free previous allocations
    if (kd_array.matrix == NULL){
        free(arr_copy);
        return NULL;
    }
    
    //Filling the matrix
    for (i=0; i<dim; i++){
        
        //creating an array of all the points' i-th coordiante
        for (j=0; j<size; j++){
            dim_coor[j].coor = spPointGetAxisCoor(arr_copy[j], i);
            dim_coor[j].index = j;
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
    return kd_array;
}

int compareKDArrayDimCoor(const void * a, const void * b){
    const struct SPKDArray* a_point_coor = (struct SPKDArray* )a;
    const struct SPKDArray* b_point_coor = (struct SPKDArray* )b;
    return (a_point_coor->coor > b_point_coor->coor ) - (a_point_coor->coor < b_point_coor->coor );
}

SPKDArray* split(SPKDArray kd_arr, int coor){
    SPKDArray* ret_array;
    int median, i, j, left_cur_pos, right_cur_pos;
    
    ret_array = (SPKDArray*)malloc(2*sizeof(SPKDArray));
    if (ret_array == NULL){
        return NULL;
    }
    
    median = (kd_arr->num)/2;
    ret_array[LEFT_INDEX]->num = (kd_arr->num - median);
    ret_array[RIGHT_INDEX]->num = median;
    
    
    //Creating the new arrays (left&right)
    for (i=0; i<=RIGHT_INDEX; i++){
        ret_array[i].arr = (SPPoint*)malloc(sizeof(SPPoint*)*ret_array[i]->num);
        
        //If the allocation failed, free all previous allocations
        if (ret_array[i].arr == NULL){
            for (j=0; j<i; j++){
                free(ret_array[j]);
            }
            free(ret_array);
            return NULL;
        }
    }
    
    left_cur_pos = 0;
    right_cur_pos = median + 1;
    for (i=0; i<kd_arr->num; i++){
        if (kd_arr->matrix[coor][i] <= median){
            ret_array[LEFT_INDEX][left_cur_pos] = kd_arr->arr[i];
            left_cur_pos++;
        } else {
            ret_array[RIGHT_INDEX][right_cur_pos] = kd_arr->arr[i];
            right_cur_pos++;
        }
    }
    
}