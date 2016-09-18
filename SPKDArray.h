#ifndef SPKDARRAY_H_
#define SPKDARRAY_H_

#include "SPPoint.h"

/**
 * This data-structure is used when building the kd-tree.
 */

typedef struct sp_kd_array *SPKDArray;

/**
 * Creates a new kd-array struct: storing relevant info,
 * sorting the points according to each coordinate and 
 * saving it to matrix.
 * 
 * @param arr - an array of SPPoints, all with the same number of coordinates.
 * @param size - the number of SPPoints in arr.
 * @return NULL if size <= 0 or dim <= 0
 * or not all the points have the same dim
 * or memory allocation failed. Otherwise return SPKDArray.
 */

SPKDArray initArray(SPPoint* arr, int size);

/**
 * Splits a kd-array into two kd-arrays, according to the median value of a given coordinate.
 * The points returned in the left kd-array would be all the points that their value in the given coor<=median,
 * the right kd-array would include only points with value>median.
 * 
 * @param kd_arr - a SPKDArray to split.
 * @param coor - which coordinate to split according to.
 * @return NULL if memory allocation failed, 
 * otherwise return an array of left and right kd-arrays.
 */

SPKDArray* split(SPKDArray kd_arr, int coor);

/**
 * Calcaulates the max spread of a specific dimenstion, 
 * that is the max value in that dimenstion minus the min value in the dimenstion.
 *
 * @param kd_arr - a SPKDArray of which we wish to calculate the spread.
 * @param coor - on which dimenstion to calculate the spread.
 * @return double (max value - min value).
 */

double getSpread(SPKDArray kd_arr, int dim);

/**
 * Calcaulates the median value of a specific dimenstion.
 *
 * @param kd_arr - a SPKDArray of which we wish to calculate the median.
 * @param coor - on which dimenstion to calculate the median.
 * @return double (the median).
 */

double getMedian(SPKDArray kd_arr, int dim);

/**
 * Gets the number of dimensions of a SPKDArray.
 * 
 * @param arr - a SPKDArray.
 * @return int (arr.dim).
 */

int getDim(SPKDArray arr);

/**
 * Gets the SPPoint array of a SPKDArray (kd-array.arr).
 *
 * @param arr - a SPKDArray.
 * @return an array of SPPoint (SPPoint*).
 */

SPPoint* getArr(SPKDArray arr);

/**
 * Free all the memory allocations the were made when the SPKDArray was created.
 * If this function is used because the initialization of the SPKDArray failed, 
 * the dimensions should be the number of dimensions that were added to the matrix before.
 *
 * @param kd_array - a SPKDArray to destroy.
 * @param dim - the number of dimensions of the SPKDArray.
 */

void destroyKdArray(SPKDArray kd_array, int dim);

/**
 * Gets the size (number of points) of a SPKDArray.
 *
 * @param kd_arr - a SPKDArray.
 * @return size (int).
 */

int getSize(SPKDArray kd_arr);

#endif /* SPKDARRAY_H_ */