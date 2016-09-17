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
#include "SPLogger.h"
}

#include <assert.h>
#include <stdio.h>

#define RETURN_FAILURE (-1)
#define RETURN_SUCCESS (0)

#define DEFAULT_CONFIG ("spcbir.config")

/* helper macro */
#define VALIDATE_SUCCESS_OR_RETURN(msg) if (msg != SP_CONFIG_SUCCESS) return MAIN_RETURN_FAILURE;

struct image_query_status {
	int index; /* image index */
	int count; /* number of time it has a close feature */
};

/* internal function - return positive val if b > a,
   negative if b < a, 0 if a == b */
int compare(const void * a, const void * b)
{
	return (((struct image_query_status*)b)->count - ((struct image_query_status*)a)->count);
}

int preprocess(SPConfig config, int numOfImages, bool isExtractionMode, sp::ImageProc & imageProc, int * featureCount, SPPoint ** features) {
	int i = 0, j = 0;
	char path[MAX_FILE_PATH]; /* buffer to store file paths */
	int ret_val = RETURN_FAILURE;
	SPPoint ** temp_features_arrays = NULL;
	int * features_count = NULL;
	int total_feautre_count = 0;

	assert(featureCount && features);

	*featureCount = 0;
	*features = NULL;

	temp_features_arrays = (SPPoint **)malloc(sizeof(SPPoint*) * numOfImages);
	if (!temp_features_arrays) goto cleanup;
	memset(temp_features_arrays, 0, sizeof(SPPoint*) * numOfImages);
	features_count = (int *)malloc(sizeof(int) * numOfImages);
	if (!features_count) goto cleanup;

	/* get the features from image files */

	for (i = 0; i < numOfImages; i++) {

		if (isExtractionMode) {
			/* Get image path from the index, and get the features */
			if (spConfigGetImagePath(path, config, i) != SP_CONFIG_SUCCESS ||
				(temp_features_arrays[i] = imageProc.getImageFeatures(path, i, features_count + i)) == NULL) {
				goto cleanup;
			}
			if (spConfigGetFeatsPath(path, config, i) != SP_CONFIG_SUCCESS || !writeImageFeatures(path, features_count[i], temp_features_arrays[i])) {
				goto cleanup;
			}
		}
		else {
			/* None extraction mode - Get feats file path from the index, and read the features */
			if (spConfigGetFeatsPath(path, config, i) != SP_CONFIG_SUCCESS ||
				(temp_features_arrays[i] = readImageFeatures(path, features_count + i)) == NULL) {
				/* fail TODO Log */
				goto cleanup;
			}
		}

		total_feautre_count += features_count[i];
	}

	
	*features = (SPPoint *)malloc(sizeof(SPPoint) * total_feautre_count);
	if (!(*features)) {
		goto cleanup;
	}

	for (i = 0; i < numOfImages; i++) {
		for (j = 0; j < features_count[i]; j++) {
			(*features)[(*featureCount)++] = temp_features_arrays[i][j];
		}
	}

	ret_val = RETURN_SUCCESS;

cleanup:
	if (temp_features_arrays) {
		for (i = 0; i < numOfImages; i++) {
			if (temp_features_arrays[i]) free(temp_features_arrays[i]);
		}
		free(temp_features_arrays);
	}
	if (ret_val != RETURN_SUCCESS && *features) {
		free(*features);
		*features = NULL;
	}
 	return ret_val;
}

/*TODO remove*/
struct sp_point_t_2{
	double* data;
	int dim;
	int index;
};


/*TODO remove first two params*/
int doQuery(SPConfig config, sp::ImageProc & imageProc, int numOfImages, bool isMinimalGui, int numOfSimilarImages, SPKDTree tree, SPBPQueue queue, bool * shouldExit) {
	char query_image[MAX_FILE_PATH];
	SPPoint * cur_image_features = NULL;
	int cur_image_num_of_feats = 0;
	int i = 0;
	unsigned int image_index = 0;
	char path[MAX_FILE_PATH];
	int path_len = 0;

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
		return true;
	}

	/* The image index here doesn't matter */
	cur_image_features = imageProc.getImageFeatures(query_image, 0, &cur_image_num_of_feats);
	/*TODO check return*/
	/* non existing file = exception ?? */

	struct image_query_status * imagesCloseFeaturesCount = (struct image_query_status *)malloc(numOfImages * sizeof(struct image_query_status));
	/* TODO check */
	for (i = 0; i < numOfImages; i++) {
		imagesCloseFeaturesCount[i].index = i;
		imagesCloseFeaturesCount[i].count = 0;
	}

	/* TODO remove the file */
	FILE *f = fopen("random.txt", "w");
	for (i = 0; i < cur_image_num_of_feats; i++) {

		kNearestNeighbors(tree, queue, cur_image_features[i]);
		/* TODO check return */

		while (!spBPQueueIsEmpty(queue)) {
			SPListElement cur_feat_elem = spBPQueuePeek(queue);
			spBPQueueDequeue(queue);
			/* TODO check */

			image_index = spListElementGetIndex(cur_feat_elem);
			imagesCloseFeaturesCount[image_index].count++;
			/*TODO: remove this print*/
			fprintf(f, "%d-%f\n", image_index, spListElementGetValue(cur_feat_elem));

			spListElementDestroy(cur_feat_elem);
		}
	}
	/* TODO remove this line */
	fclose(f);
	
	/* sort imagesCloseFeaturesCount array in descending order */

	qsort(imagesCloseFeaturesCount, numOfImages, sizeof(image_query_status), compare);

	if (!isMinimalGui) {
		printf("Best candidates for - %s - are:\n", query_image);
	}

	for (i = 0; i < numOfSimilarImages && i < numOfImages; i++) {
		spConfigGetImagePath(path, config, imagesCloseFeaturesCount[i].index);
		/*TODO check return */
		if (isMinimalGui) {
			imageProc.showImage(path);
		}
		else {
			printf("%s\n", path);
		}
	}
	
	/* TODO free imagesCloseFeaturesCount */

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
		printf("The %sconfiguration file %s couldn't be open\n", ((char*)config_filename == (char*)DEFAULT_CONFIG ? "default " : ""), config_filename);
	}
	if (res_msg != SP_CONFIG_SUCCESS) return RETURN_FAILURE;

	/* init logger */
	spConfigGetLoggerFilename(logger_filename, config); /* through this module, we will not check config-get function */
														/* because their only possible failure is invalid argument */
	if (spLoggerCreate(logger_filename, (SP_LOGGER_LEVEL)spConfigGetLoggerLevel(config, &res_msg)) != SP_LOGGER_SUCCESS) {
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
	
	/* TODO remove the cast!! */
	tree = initTree(features, numOfFeatures, spConfigGetKDTreeSplitMethod(config, &res_msg));
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