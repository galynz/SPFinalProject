#ifndef SPCONFIG_H_
#define SPCONFIG_H_

#include "common.h"

#include <stdbool.h>
#include <stdio.h>

/**
 * A data-structure which is used for configuring the system.
 */

typedef enum sp_config_msg_t {
	SP_CONFIG_MISSING_DIR,
	SP_CONFIG_MISSING_PREFIX,
	SP_CONFIG_MISSING_SUFFIX,
	SP_CONFIG_MISSING_NUM_IMAGES,
    SP_CONFIG_MISSING_NON_DEFAULT_VALUE,
	SP_CONFIG_CANNOT_OPEN_FILE,
	SP_CONFIG_ALLOC_FAIL,
	SP_CONFIG_INVALID_INTEGER,
	SP_CONFIG_INVALID_STRING,
	SP_CONFIG_INVALID_ARGUMENT,
	SP_CONFIG_INDEX_OUT_OF_RANGE,
    SP_CONFIG_INVALID_LINE,
    SP_CONFIG_CONSTRAINT_NOT_MET,
	SP_CONFIG_SUCCESS
} SP_CONFIG_MSG;

typedef struct sp_config_t* SPConfig;

/**
 * Creates a new system configuration struct. The configuration struct
 * is initialized based on the configuration file given by 'filename'.
 * 
 * @param filename - the name of the configuration file
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return NULL in case an error occurs. Otherwise, a pointer to a struct which
 * 		   contains all system configuration.
 * 
 * The resulting value stored in msg is as follow:
 * - SP_CONFIG_INVALID_ARGUMENT - if filename == NULL
 * - SP_CONFIG_CANNOT_OPEN_FILE - if the configuration file given by filename cannot be open
 * - SP_CONFIG_ALLOC_FAIL - if an allocation failure occurred
 * - SP_CONFIG_INVALID_INTEGER - if a line in the config file contains invalid integer
 * - SP_CONFIG_INVALID_STRING - if a line in the config file contains invalid string
 * - SP_CONFIG_MISSING_DIR - if spImagesDirectory is missing
 * - SP_CONFIG_MISSING_PREFIX - if spImagesPrefix is missing
 * - SP_CONFIG_MISSING_SUFFIX - if spImagesSuffix is missing 
 * - SP_CONFIG_MISSING_NUM_IMAGES - if spNumOfImages is missing
 * - SP_CONFIG_SUCCESS - in case of success
 *
 *
 */
SPConfig spConfigCreate(const char* filename, SP_CONFIG_MSG* msg);

/*
 * Returns true if spExtractionMode = true, false otherwise.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if spExtractionMode = true, false otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
bool spConfigIsExtractionMode(const SPConfig config, SP_CONFIG_MSG* msg);

/*
 * Returns true if spMinimalGUI = true, false otherwise.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return true if spExtractionMode = true, false otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
bool spConfigMinimalGui(const SPConfig config, SP_CONFIG_MSG* msg);

/*
 * Returns the number of images set in the configuration file, i.e the value
 * of spNumOfImages.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return positive integer in success, negative integer otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
int spConfigGetNumOfImages(const SPConfig config, SP_CONFIG_MSG* msg);

/*
* Returns the number of similar images set in the configuration file, i.e the value
* of spNumOfSimilarImages.
*
* @param config - the configuration structure
* @assert msg != NULL
* @param msg - pointer in which the msg returned by the function is stored
* @return positive integer in success, negative integer otherwise.
*
* - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
* - SP_CONFIG_SUCCESS - in case of success
*/
int spConfigGetNumOfSimilarImages(const SPConfig config, SP_CONFIG_MSG* msg);

/*
* Returns the KNN set in the configuration file, i.e the value
* of spKNN.
*
* @param config - the configuration structure
* @assert msg != NULL
* @param msg - pointer in which the msg returned by the function is stored
* @return positive integer in success, negative integer otherwise.
*
* - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
* - SP_CONFIG_SUCCESS - in case of success
*/
int spConfigGetKNN(const SPConfig config, SP_CONFIG_MSG* msg);

/*
 * Returns the number of features to be extracted. i.e the value
 * of spNumOfFeatures.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return positive integer in success, negative integer otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
int spConfigGetNumOfFeatures(const SPConfig config, SP_CONFIG_MSG* msg);

/**
 * Returns the dimension of the PCA. i.e the value of spPCADimension.
 *
 * @param config - the configuration structure
 * @assert msg != NULL
 * @param msg - pointer in which the msg returned by the function is stored
 * @return positive integer in success, negative integer otherwise.
 *
 * - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
 * - SP_CONFIG_SUCCESS - in case of success
 */
int spConfigGetPCADim(const SPConfig config, SP_CONFIG_MSG* msg);

/**
* Returns the the kd-tree split method set by the config. i.e the value of spKDTreeSplitMethod.
*
* @param config - the configuration structure
* @assert msg != NULL
* @param msg - pointer in which the msg returned by the function is stored
* @return one of {MAX_SPREAD, RANDOM, INCREMENTAL}, negative integer otherwise.
*
* - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
* - SP_CONFIG_SUCCESS - in case of success
*/
KDTREE_SPLIT_METHOD spConfigGetKDTreeSplitMethod(const SPConfig config, SP_CONFIG_MSG* msg);

/**
 * Given an index 'index' the function stores in imagePath the full path of the
 * ith image.
 *
 * For example:
 * Given that the value of:
 *  spImagesDirectory = "./images/"
 *  spImagesPrefix = "img"
 *  spImagesSuffix = ".png"
 *  spNumOfImages = 17
 *  index = 10
 *
 * The functions stores "./images/img10.png" to the address given by imagePath.
 * Thus the address given by imagePath must contain enough space to
 * store the resulting string.
 *
 * @param imagePath - an address to store the result in, it must contain enough space.
 * @param config - the configuration structure
 * @param index - the index of the image.
 *
 * @return
 * - SP_CONFIG_INVALID_ARGUMENT - if imagePath == NULL or config == NULL
 * - SP_CONFIG_INDEX_OUT_OF_RANGE - if index >= spNumOfImages
 * - SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG spConfigGetImagePath(char* imagePath, const SPConfig config,
		int index);

/**
* Given an index 'index' the function stores in featsPath the full path of the
* ith image features file.
*
* @param featsPath - an address to store the result in, it must contain enough space.
* @param config - the configuration structure
* @param index - the index of the image.
*
* @return
* - SP_CONFIG_INVALID_ARGUMENT - if imagePath == NULL or config == NULL
* - SP_CONFIG_INDEX_OUT_OF_RANGE - if index >= spNumOfImages
* - SP_CONFIG_SUCCESS - in case of success
*/
SP_CONFIG_MSG spConfigGetFeatsPath(char* featsPath, const SPConfig config,
	int index)

/**
 * The function stores in pcaPath the full path of the pca file.
 * For example given the values of:
 *  spImagesDirectory = "./images/"
 *  spPcaFilename = "pca.yml"
 *
 * The functions stores "./images/pca.yml" to the address given by pcaPath.
 * Thus the address given by pcaPath must contain enough space to
 * store the resulting string.
 *
 * @param imagePath - an address to store the result in, it must contain enough space.
 * @param config - the configuration structure
 * @return
 *  - SP_CONFIG_INVALID_ARGUMENT - if imagePath == NULL or config == NULL
 *  - SP_CONFIG_SUCCESS - in case of success
 */
SP_CONFIG_MSG spConfigGetPCAPath(char* pcaPath, const SPConfig config);

/*
* Returns the Logger level set in the configuration file, i.e the value
* of spLoggerLevel.
*
* @param config - the configuration structure
* @assert msg != NULL
* @param msg - pointer in which the msg returned by the function is stored
* @return one of {1, 2, 3, 4}, negative integer otherwise.
*
* - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
* - SP_CONFIG_SUCCESS - in case of success
*/
int spConfigGetLoggerLevel(const SPConfig config, SP_CONFIG_MSG* msg);

/**
* The function stores in loggerFilename the name of the log file set by the config.
*
* @param loggerFilename - an address to store the result in, it must contain enough space.
* @param config - the configuration structure
* @return
*  - SP_CONFIG_INVALID_ARGUMENT - if imagePath == NULL or config == NULL
*  - SP_CONFIG_SUCCESS - in case of success
*/
SP_CONFIG_MSG spConfigGetLoggerFilename(char* loggerFilename, const SPConfig config);

/**
 * Frees all memory resources associate with config. 
 * If config == NULL nothig is done.
 */
void spConfigDestroy(SPConfig config);

#endif /* SPCONFIG_H_ */
