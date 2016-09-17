/*TODO remove*/
#if defined ( WIN32 )
#include "SPImageProc2.h"
#else

#include "SPImageProc.h"

/*TODO remove*/
#endif

extern "C" {
#include "SPConfig.h"
#include "SPKDTree.h"
#include "SPBPriorityQueue.h"
#include "common.h"
#include "FeaturesStorage.h"
}

#include <assert.h>
#include <stdio.h>

#define RETURN_FAILURE (-1)
#define RETURN_SUCCESS (0)

#define DEFAULT_CONFIG ("spcbir.config")

struct image_query_status {
	int index; /* image index */
	int count; /* number of time it has a close feature */
};

/* internal function - return positive val if b > a,
   negative if b < a, 0 if a == b */
static int compare_image_query_status(const void * a, const void * b)
{
	return (((struct image_query_status*)b)->count - ((struct image_query_status*)a)->count);
}

/* internal function, loads all features from the db.
Params:
   config - the SPConfig state object
   numOfImages - number of images in the db 
   isExtraction - whether images rather than .feats files should be processed
   imageProc - a reference to the ImageProc class
   featureCount - a pointer to a variable that will hold the number of extracted features
   features - the result, a pointer to an array that will contain all the features
Returns:
	RETURN_SUCCESS on success, RETURN_FAILURE otherwise
*/
static int preprocess(SPConfig config, int numOfImages, bool isExtractionMode, sp::ImageProc & imageProc, int * featureCount, SPPoint ** features) {
	int i = 0, j = 0;
	char path[MAX_FILE_PATH]; /* buffer to store file paths */
	int ret_val = RETURN_FAILURE;
	SPPoint ** temp_features_arrays = NULL;
	int * features_count = NULL;
	int total_feautre_count = 0;

	assert(config && featureCount && features);

	*featureCount = 0;
	*features = NULL;
	
	features_count = (int *)malloc(sizeof(int) * numOfImages);
	VALIDATE_WITH_LOG(features_count, "Allocation error");
	memset(features_count, 0, sizeof(int) * numOfImages);
	temp_features_arrays = (SPPoint **)malloc(sizeof(SPPoint*) * numOfImages);
	VALIDATE_WITH_LOG(temp_features_arrays, "Allocation error");
	memset(temp_features_arrays, 0, sizeof(SPPoint*) * numOfImages);

	/* get the features from image/feat files */

	for (i = 0; i < numOfImages; i++) {

		if (isExtractionMode) {
			/* Get image path from the index, and get the features */
			VALIDATE_WITH_LOG(spConfigGetImagePath(path, config, i) == SP_CONFIG_SUCCESS, "Error getting image path");
			temp_features_arrays[i] = imageProc.getImageFeatures(path, i, features_count + i);
			VALIDATE_WITH_LOG(temp_features_arrays[i], "Error reading image features");
			
			/* save the read features to a .feat file*/
			VALIDATE_WITH_LOG(spConfigGetFeatsPath(path, config, i) == SP_CONFIG_SUCCESS, "Error getting .feats file path");
			VALIDATE_WITH_LOG(writeImageFeatures(path, features_count[i], temp_features_arrays[i]), "Error writing image features");
		}
		else {
			/* None extraction mode - Get feats file path from the index, and read the features */
			VALIDATE_WITH_LOG(spConfigGetFeatsPath(path, config, i) == SP_CONFIG_SUCCESS, "Error getting .feats file path");
			temp_features_arrays[i] = readImageFeatures(path, features_count + i);
			VALIDATE_WITH_LOG(temp_features_arrays[i], "Error reading features from .feats file");
		}

		total_feautre_count += features_count[i];
	}

	
	*features = (SPPoint *)malloc(sizeof(SPPoint) * total_feautre_count);
	VALIDATE_WITH_LOG(*features, "Allocation error");

	/* copy all the features to one big array */
	for (i = 0; i < numOfImages; i++) {
		for (j = 0; j < features_count[i]; j++) {
			(*features)[(*featureCount)++] = temp_features_arrays[i][j];
			temp_features_arrays[i][j] = NULL;
		}
	}

	ret_val = RETURN_SUCCESS;

cleanup:
	if (temp_features_arrays) {
		for (i = 0; i < numOfImages; i++) {
			if (ret_val != RETURN_SUCCESS) {
				for (j = 0; j < features_count[i]; j++) {
					if (temp_features_arrays[i][j]) spPointDestroy(temp_features_arrays[i][j]);
				}
			}
			if (temp_features_arrays[i]) free(temp_features_arrays[i]);
		}
		free(temp_features_arrays);
	}
	if (features_count) free(features_count);
	if (ret_val != RETURN_SUCCESS && *features) {
		free(*features);
		*features = NULL;
	}
 	return ret_val;
}

/* Asks the user to enter an image path, checks which images are closest to it, and displays the results.
Params:
	config - SPConfig object
	imageProc - reference to ImageProc class
	numOfImages - the number of images in the DB
	isMinimalGui - whether or not the running mode is 'minimal gui'
	numOfSimilarImages - the number of 'candidates' that should be displayed
	tree - a KD-Tree object that is initialized with the features of all images
	queue - a SPBPQueue object, that its size is the 'KNN' value
	shouldExit - a pointer to a bool var that will indicate whether to user asked to terminate
Return:
	RETURN_SUCCESS on success, RETURN_FAILURE otherwise
*/
static int doQuery(SPConfig config, sp::ImageProc & imageProc, int numOfImages, bool isMinimalGui, int numOfSimilarImages, SPKDTree tree, SPBPQueue queue, bool * shouldExit) {
	char query_image[MAX_FILE_PATH];
	SPPoint * cur_image_features = NULL;
	int cur_image_num_of_feats = 0;
	int i = 0;
	unsigned int image_index = 0;
	char path[MAX_FILE_PATH];
	int path_len = 0;
	struct image_query_status * imagesCloseFeaturesCount = NULL;
	int ret_val = RETURN_FAILURE;
	SPListElement cur_feat_elem = NULL;

	assert(config && tree && queue && shouldExit);
	
	*shouldExit = false;

	printf("Please enter an image path:\n");
	fgets(query_image, sizeof(query_image), stdin);
	path_len = strlen(query_image);
	if (query_image[path_len - 1] == '\n') {
		query_image[path_len - 1] = '\0';
	}
	if (strcmp("<>", query_image) == 0) {
		*shouldExit = true;
		return RETURN_SUCCESS;
	}

	/* Extract query features (The image index here doesn't matter) */
	cur_image_features = imageProc.getImageFeatures(query_image, 0, &cur_image_num_of_feats);
	VALIDATE_WITH_LOG(cur_image_features, "Bad image path or error extracting features");

	/* Create helper counter array - keeps track of how many images have had 'hits' */
	imagesCloseFeaturesCount = (struct image_query_status *)malloc(numOfImages * sizeof(struct image_query_status));
	VALIDATE_WITH_LOG(imagesCloseFeaturesCount, "Allocation error");
	for (i = 0; i < numOfImages; i++) {
		imagesCloseFeaturesCount[i].index = i;
		imagesCloseFeaturesCount[i].count = 0;
	}

	/* go through all query image features */
	for (i = 0; i < cur_image_num_of_feats; i++) {
		/* search for the closest features to current feature */
		VALIDATE_WITH_LOG(kNearestNeighbors(tree, queue, cur_image_features[i]),
			"Error searching for close features");

		/* advance hit counters */
		while (!spBPQueueIsEmpty(queue)) {
			cur_feat_elem = spBPQueuePeek(queue);
			VALIDATE_WITH_LOG(cur_feat_elem, "Error picking queue");

			VALIDATE_WITH_LOG(spBPQueueDequeue(queue) == SP_BPQUEUE_SUCCESS, "Error dequeueing");
			
			image_index = spListElementGetIndex(cur_feat_elem);
			imagesCloseFeaturesCount[image_index].count++;

			spListElementDestroy(cur_feat_elem);
		}
	}
	
	/* sort hits counter array in descending order */
	qsort(imagesCloseFeaturesCount, numOfImages, sizeof(image_query_status), compare_image_query_status);

	/* display results */
	if (!isMinimalGui) printf("Best candidates for - %s - are:\n", query_image);
	for (i = 0; i < numOfSimilarImages && i < numOfImages; i++) {
		VALIDATE_WITH_LOG(spConfigGetImagePath(path, config, imagesCloseFeaturesCount[i].index), "Error getting image path");
		if (isMinimalGui) {
			/* if it's gui mode - display the image */
			imageProc.showImage(path);
		}
		else {
			/* otherwise - print it's path */
			printf("%s\n", path);
		}
	}
	
	ret_val = RETURN_SUCCESS;
	
cleanup:
	if (cur_image_features) {
		for (i = 0; i < cur_image_num_of_feats; i++) {
			spPointDestroy(cur_image_features[i]);
		}
		free(cur_image_features);
	}
	if (imagesCloseFeaturesCount) free(imagesCloseFeaturesCount);

	return ret_val;
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
	int i = 0;
	
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
		printf("The %sconfiguration file %s couldn't be open\n", ((char*)config_filename == (char*)DEFAULT_CONFIG ? "default " : ""), config_filename);
	}
	if (res_msg != SP_CONFIG_SUCCESS) return RETURN_FAILURE;

	/* init logger */
	spConfigGetLoggerFilename(logger_filename, config); /* through this module, we will not check config-get function */
														/* because their only possible failure is invalid argument */
	if (spLoggerCreate((strcmp(logger_filename, "stdout") == 0 ? NULL : logger_filename),
		(SP_LOGGER_LEVEL)spConfigGetLoggerLevel(config, &res_msg)) != SP_LOGGER_SUCCESS) {
		goto cleanup;
	}
	logger_initilized = true;

	imageProc = new sp::ImageProc(config);
	
	numOfImages = spConfigGetNumOfImages(config, &res_msg);
	isMinimalGui = spConfigMinimalGui(config, &res_msg);
	numOfSimilarImages = spConfigGetNumOfSimilarImages(config, &res_msg);

	/* Preprocess - Load all features */
	if (RETURN_SUCCESS != preprocess(config, numOfImages, spConfigIsExtractionMode(config, &res_msg), *imageProc, &numOfFeatures, &features)) {
		goto cleanup;
	}
	
	tree = initTree(features, numOfFeatures, spConfigGetKDTreeSplitMethod(config, &res_msg));
	VALIDATE_WITH_LOG(tree, "Error creating kd-tree");

	queue = spBPQueueCreate(spConfigGetKNN(config, &res_msg));
	VALIDATE_WITH_LOG(queue, "Error creating BPQueue");

	/* main loop - ask for query images from user, and display results */
	while (!shouldExit) {
		if (RETURN_SUCCESS != doQuery(config, *imageProc, numOfImages, isMinimalGui, numOfSimilarImages, tree, queue, &shouldExit)) {
			goto cleanup;
		}
	}

	printf("Exiting...\n");
	ret_val = RETURN_SUCCESS;
	
cleanup:

	if (queue) spBPQueueDestroy(queue);
	if (tree) destroyTree(tree);
	if (features) {
		for (i = 0; i < numOfFeatures; i++) {
			spPointDestroy(features[i]);
		}
		free(features);
	}
	if (imageProc) delete imageProc;
	if (logger_initilized) spLoggerDestroy();
	spConfigDestroy(config);

	return ret_val;
}