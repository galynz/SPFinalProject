#ifndef SPKDARRAY_H_
#define SPKDARRAY_H_

#include "SPPoint.h"

typedef struct sp_kd_array *SPKDArray;

SPKDArray init(SPPoint* arr, int size);

SPKDArray* split(SPKDArray kd_arr, int coor);

double get_spread(SPKDArray kd_arr, int dim);

double get_median(SPKDArray kd_arr, int dim);

int get_dim(SPKDArray arr);

SPPoint* get_arr(SPKDArray arr);

void destroy_kd_array(SPKDArray kd_array, int dim);

int get_size(SPKDArray kd_arr);

#endif /* SPKDARRAY_H_ */