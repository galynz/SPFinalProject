#ifndef FEATURESSTORAGE_H_
#define FEATURESSTORAGE_H_

#include "SPPoint.h"

/**
* Reads an array of features from the file at featsPath. The number of features
* extracted will be stored in the pointer given by numOfFeats.
*
* @param featsPath - the feats file
* @param numOfFeats - a pointer in which the actual number of feats extracted
* 					   will be stored
* @return
* An array of features extracted. NULL is returned in case of an error.
*/
SPPoint* readImageFeatures(const char* featsPath, int* numOfFeats);

/**
* Writes an array of features to featsPath.
*
* @param featsPath - the feats file to be written
* @param numOfFeats - number of features in the array
* @param features - array of features
* @return
* true in case of success, false on failure
*/
bool writeImageFeatures(const char* featsPath, int numOfFeats, SPPoint* features);

#endif /*FEATURESSTORAGE_H_*/