#include "SPPoint.h"
#include <assert.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>

struct sp_point_t{
   double* data;
   int dim;
   int index;
};

SPPoint spPointCreate(double* data, int dim, int index){
    // We rely on the SPPoint copy function to do all allocations
    struct sp_point_t point = {data, dim, index};
    // Verifying the input
    if (data == NULL || dim <= 0 || index < 0){
        return NULL;
    }
    return spPointCopy(&point);
}

SPPoint spPointCopy(SPPoint source){
    SPPoint point = NULL;
    // Assertions
    assert(source != NULL);
    // Allocate a pointer to new point
    point = (SPPoint)malloc(sizeof(struct sp_point_t));
    if (point == NULL){
        return NULL;
    }
    // Allocate memory for the data
    point->data = (double*)malloc(source->dim*sizeof(double));
    if (point->data == NULL){
        // If the allocation failed, free previous allocations
        free(point);
        return NULL;
    }

    for (int i = 0; i < source->dim; i++) {
        point->data[i] = source->data[i];
    }

    point->dim = source->dim;
    point->index = source->index;
    return point;
}

void spPointDestroy(SPPoint point) {
    if (point != NULL){
        // Making sure that data is not null before freeing it
        if (point->data != NULL){
            free(point->data);
        }
        free(point);
    }
}

int spPointGetDimension(SPPoint point) {
    assert(point != NULL);
    return point->dim;
}

int spPointGetIndex(SPPoint point) {
    assert(point != NULL);
    return point->index;
}

double spPointGetAxisCoor(SPPoint point, int axis) {
    assert(point != NULL && point->dim > axis && axis >= 0);
    return point->data[axis];
}

double spPointL2SquaredDistance(SPPoint p, SPPoint q){
    double distance = 0;
    assert( p != NULL && q != NULL && p->dim == q->dim);
    // Calculating the L2-squared distance between p and q
    for (int i=0; i<p->dim; i++){
        // Calculating (p_i - q_i)^2
        distance += (p->data[i] - q->data[i])*(p->data[i] - q->data[i]);
    }
    return distance;
}
    
int spPointSerialize(SPPoint p, char ** data) {
    int needed_size = 0;
    int * int_write_ptr = NULL;
    assert(p != NULL && data != NULL);
    
    /* We need space for the dim & index, and for the data */
    needed_size = 2 * sizeof(int) + p->dim*sizeof(double);
    
    *data = (char *)malloc(needed_size);
    if (!*data) {
        return -1; /* allocation error */
    }
    
    int_write_ptr = (int *)*data;
    
    *(int_write_ptr++) = p->dim;
    *(int_write_ptr++) = p->index;
    memcpy(int_write_ptr, p->data, p->dim*sizeof(double));
    
    return needed_size;
}

SPPoint spPointDeserialize(char * data, char ** end_of_data) {
    int * int_read_ptr = (int *)data;
    int dim = 0, index = 0;
    char * point_data = NULL;
    assert(data != NULL && end_of_data != NULL);
    
    dim = *(int_read_ptr++);
    index = *(int_read_ptr++);
    point_data = (char *)int_read_ptr;
    
	*end_of_data = point_data + dim*sizeof(double);
    
	return spPointCreate((double *)point_data, dim, index);
}
