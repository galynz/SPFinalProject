#include "SPImageProc.h"

extern "C" {
#include "SPConfig.h"
#include "SPKDTree.h"
#include "SPBPriorityQueue.h"
#include "common.h"
#include "FeaturesStorage.h"
#include "SPLogger.h"
}

#include <assert.h>
#include <stdio.h>

#define RETURN_FAILURE (-1)
#define RETURN_SUCCESS (0)

#define DEFAULT_CONFIG ("spcbir.config")

/* helper macro */
#define VALIDATE_SUCCESS_OR_RETURN(msg) if (msg != SP_CONFIG_SUCCESS) return MAIN_RETURN_FAILURE;

/* internal function - return positive val if b > a,
   negative if b < a, 0 if a == b */
int compare(const void * a, const void * b)
{
	return (*(int*)b - *(int*)a);
}

int preprocess(SPConfig config, int numOfImages, bool isExtractionMode, int maxNumOfFeatures, sp::ImageProc & imageProc, int * featureCount, SPPoint ** features) {
	int i = 0;
	SPPoint * cur_image_features = NULL;
	int cur_image_num_of_feats = 0;
	char path[MAX_FILE_PATH]; /* buffer to store file paths */
	int ret_val = RETURN_FAILURE;

	assert(featureCount && features);

	*featureCount = 0;
	*features = NULL;

	*features = (SPPoint *)malloc(sizeof(SPPoint) * numOfImages * maxNumOfFeatures);
	if (!(*features)) {
		goto cleanup;
	}

	/* get the features from image files */

	for (i = 0; i < numOfImages; i++) {

		if (isExtractionMode) {
			/* Get image path from the index, and get the features */
			if (spConfigGetImagePath(path, config, i) != SP_CONFIG_SUCCESS ||
				(cur_image_features = imageProc.getImageFeatures(path, i, &cur_image_num_of_feats)) == NULL) {
				goto cleanup;
			}
			if (!writeImageFeatures(path, cur_image_num_of_feats, cur_image_features)) {
				free(cur_image_features);
				goto cleanup;
			}
		}
		else {
			/* None extraction mode - Get feats file path from the index, and read the features */
			if (spConfigGetFeatsPath(path, config, i) != SP_CONFIG_SUCCESS ||
				(cur_image_features = readImageFeatures(path, &cur_image_num_of_feats)) == NULL) {
				/* fail TODO Log */
				goto cleanup;
			}
		}

		/* copy all features to the features array */
		while (cur_image_num_of_feats) {
			(*features)[(*featureCount)++] = cur_image_features[--cur_image_num_of_feats];
		}

		free(cur_image_features);
	}

	ret_val = RETURN_SUCCESS;

cleanup:
	if (ret_val != RETURN_SUCCESS && *features) {
		free(*features);
		*features = NULL;
	}
 	return ret_val;
}

int doQuery(SPConfig config, sp::ImageProc & imageProc, int numOfImages, bool isMinimalGui, int numOfSimilarImages, SPKDTree tree, SPBPQueue queue, bool * shouldExit) {
	char query_image[MAX_FILE_PATH];
	SPPoint * cur_image_features = NULL;
	int cur_image_num_of_feats = 0;
	int i = 0;
	unsigned int image_index = 0;
	char path[MAX_FILE_PATH];

	assert(config && tree && queue && shouldExit);
	
	*shouldExit = false;

	printf("Please enter an image path:\n");
	fgets(query_image, sizeof(query_image), stdin);
	if (strcmp("<>", query_image) == 0) {
		*shouldExit = true;
		return true;
	}

	/* The image index here doesn't matter */
	cur_image_features = imageProc.getImageFeatures(query_image, 0, &cur_image_num_of_feats);
	/*TODO check return*/
	/* non existing file = exception ?? */

	for (i = 0; i < cur_image_num_of_feats; i++) {

		kNearestNeighbors(tree, queue, cur_image_features[i]);
		/* TODO check return */

		int * imagesCloseFeaturesCount = (int *)malloc(numOfImages * sizeof(int));
		/* TODO check */
		memset(imagesCloseFeaturesCount, 0, numOfImages * sizeof(int));

		while (!spBPQueueIsEmpty(queue)) {
			SPListElement cur_feat_elem = spBPQueuePeek(queue);
			spBPQueueDequeue(queue);
			/* TODO check */

			image_index = spListElementGetIndex(cur_feat_elem);
			imagesCloseFeaturesCount[image_index]++;

			spListElementDestroy(cur_feat_elem);
		}

		/* sort imagesCloseFeaturesCount array in descending order */
		qsort(imagesCloseFeaturesCount, numOfImages, sizeof(int), compare);

		if (!isMinimalGui) {
			printf("Best candidates for - %s - are:\n", query_image);
		}

		for (i = 0; i < numOfSimilarImages && i < numOfImages; i++) {
			spConfigGetImagePath(path, config, i);
			/*TODO check return */
			if (isMinimalGui) {
				imageProc.showImage(path);
			}
			else {
				printf("%s\n", path);
			}
		}
	}

	free(cur_image_features);

	return RETURN_SUCCESS;
}

int main(int argc, char * argv[]) {
	const char * config_filename = DEFAULT_CONFIG;
	SP_CONFIG_MSG res_msg = SP_CONFIG_SUCCESS;
	SPConfig config = NULL;
	SPPoint * features = NULL;
	int numOfFeatures = 0, numOfImages = 0, numOfSimilarImages = 0;
	bool isMinimalGui = false, shouldExit = false, logger_initilized = false;;
	char logger_filename[MAX_FILE_PATH];
	SPKDTree tree = NULL;
	SPBPQueue queue = NULL;
	int ret_val = RETURN_FAILURE;
	sp::ImageProc * imageProc = NULL;
	
	if (argc != 1) { /* if there are any arguments */
		if (argc != 3 || strcmp(argv[1], "-c") != 0) {  /* if unexpected arguments */
			printf("Invalid command line : use -c <config_filename>\n");
			return RETURN_FAILURE;
		}
		config_filename = argv[2]; /* change from default */
	}

	/* load the config */
	config = spConfigCreate(config_filename, &res_msg);
	if (res_msg == SP_CONFIG_CANNOT_OPEN_FILE) {
		printf("The %sconfiguration file %s couldn’t be open\n", ((char*)config_filename == (char*)DEFAULT_CONFIG ? "default " : ""), config_filename);
	}
	if (res_msg != SP_CONFIG_SUCCESS) return RETURN_FAILURE;

	/* init logger */
	spConfigGetLoggerFilename(logger_filename, config); /* through this module, we will not check config-get function */
														/* because their only possible failure is invalid argument */
	if (spLoggerCreate(logger_filename, (SP_LOGGER_LEVEL)spConfigGetLoggerLevel(config, &res_msg)) != SP_LOGGER_SUCCESS) {
		goto cleanup;
	}
	logger_initilized = true;

	imageProc = new sp::ImageProc(config); /* C++ - so it's ok to define vars not at the beginning */
	
	numOfImages = spConfigGetNumOfImages(config, &res_msg);
	isMinimalGui = spConfigMinimalGui(config, &res_msg);
	numOfSimilarImages = spConfigGetNumOfSimilarImages(config, &res_msg);

	/* Preprocess - Load all features */
	if (RETURN_SUCCESS != preprocess(config, numOfImages, spConfigIsExtractionMode(config, &res_msg), spConfigGetNumOfFeatures(config, &res_msg), *imageProc, &numOfFeatures, &features)) {
		goto cleanup;
	}
	
	/* TODO remove the cast!! */
	tree = initTree(features, numOfFeatures, (SplitMethod)spConfigGetKDTreeSplitMethod(config, &res_msg));
	/* TODO validate tree */

	queue = spBPQueueCreate(spConfigGetKNN(config, &res_msg));
	/* TODO check */

	/* main loop - ask for query images from user, and display results */
	while (!shouldExit) {
		if (RETURN_SUCCESS != doQuery(config, *imageProc, numOfImages, isMinimalGui, numOfSimilarImages, tree, queue, &shouldExit)) {
			goto cleanup;
		}
	}

	ret_val = RETURN_SUCCESS;
	
cleanup:

	if (queue) spBPQueueDestroy(queue);
	if (tree) destroyTree(tree);
	if (features) free(features);
	if (imageProc) delete imageProc;
	if (logger_initilized) spLoggerDestroy();
	spConfigDestroy(config);

	return ret_val;
}