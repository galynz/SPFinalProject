#include "SPConfig.h"
#include "SPImageProc.h"
#include "SPKDTree.h"
#include "SPBPriorityQueue.h"
#include "common.h"
#include "FeaturesStorage.h"

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

int preprocess(SPConfig config, size_t numOfImages, bool isExtractionMode, int maxNumOfFeatures, sp::ImageProc & imageProc, int * featureCount, SPPoint ** features) {
	int i = 0;
	SPPoint * cur_image_features = NULL;
	int cur_image_num_of_feats = 0;
	char path[MAX_FILE_PATH]; /* buffer to store file paths */
	bool extractionSuccess = false;

	assert(featureCount && features);
	
	*featureCount = 0;

	*features = (SPPoint *)malloc(sizeof(SPPoint) * numOfImages * maxNumOfFeatures);
	if (!features) {
		return RETURN_FAILURE;
	}

	/* get the features from image files */
	
	for (i = 0; i < numOfImages; i++) {
	
		if ((isExtractionMode &&  /* Get image path from the index, and get the features */
			(spConfigGetImagePath(path, config, i) != SP_CONFIG_SUCCESS ||
			(cur_image_features = imageProc.getImageFeatures(path, i, &cur_image_num_of_feats)) == NULL)) ||
			/* None extraction mode - Get feats file path from the index, and read the features */
			(spConfigGetFeatsPath(path, config, i) != SP_CONFIG_SUCCESS ||
			(cur_image_features = readImageFeatures(path, &cur_image_num_of_feats)) == NULL)) {
			/* fail TODO Log */
			free(*features);
			*features = NULL;
			return RETURN_FAILURE;
		}

		while (cur_image_num_of_feats) {
			(*features)[(*featureCount)++] = cur_image_features[--cur_image_num_of_feats];
		}

		free(cur_image_features);
	}
	
	return RETURN_SUCCESS;
}

int main(int argc, char * argv[]) {
	const char * config_filename = DEFAULT_CONFIG;
	SP_CONFIG_MSG res_msg = SP_CONFIG_SUCCESS;
	SPConfig config = NULL;
	int * numOfFeat = NULL;
	SPPoint ** features = NULL;
	char query_image[MAX_FILE_PATH];

	/* if there are some arguments */
	if (argc != 1) {
		if (argc != 3 || strcmp(argv[1], "-c") != 0) {  /* unexpected arguments */
			printf("Invalid command line : use -c <config_filename>\n");
			return MAIN_RETURN_FAILURE;
		}
		config_filename = argv[2];
	}

	/* load the config */
	config = spConfigCreate(config_filename, &res_msg);
	if (res_msg == SP_CONFIG_CANNOT_OPEN_FILE) {
		printf("The %sconfiguration file %s couldn’t be open\n",
			(config_filename == DEFAULT_CONFIG ? "default " : ""), config_filename);
	}
	VALIDATE_SUCCESS_OR_RETURN(res_msg);

	/*TODO init logger*/

	sp::ImageProc imageProc(config);

	if (preprocess(config, ) != RETURN_SUCCESS) {

	}


	tree = initTree(features, feature_count, method);
	/* TODO validate tree */

	queue = spBPQueueCreate(kdd);
	/* TODO check */

	while (true) {
		printf("Please enter an image path:\n");
		gets(query_image);
		if (strcmp("<>", query_image) == 0) {
			break; /* exit */
		}

		/* The image index here doesn't matter */
		cur_image_features = imageProc.getImageFeatures(query_image, 0, &cur_image_num_of_feats);
		/*TODO check return*/
		/* non existing file = exception ?? */

		for (i = 0; i < cur_image_num_of_feats; i++) {
			
			kNearestNeighbors(tree, queue, cur_image_num_of_feats[i]);
			/* TODO check return */

			int * imagesCloseFeaturesCount = malloc(numofImages * sizeof(int));
			/* TODO check */
			memset(imagesCloseFeaturesCount, 0, numofImages * sizeof(int));

			while (!spBPQueueIsEmpty(queue)) {
				SPListElement cur_feat_elem = spBPQueuePeek(queue);
				spBPQueueDequeue(queue);
				/* TODO check */

				unsigned int index = spListElementGetIndex(cur_feat_elem);
				imagesCloseFeaturesCount[index]++;

				spListElementDestroy(cur_feat_elem);
			}

			/* sort imagesCloseFeaturesCount array in descending order */
			qsort(imagesCloseFeaturesCount, numOfImages, sizeof(int), compare);

			if (!minimalGui) {
				printf("Best candidates for - %s - are:\n", query_image);
			}

			for (i = 0; i < numOfSimilarImages && i < numOfImages; i++) {
				spConfigGetImagePath(path, config, i);
				/*TODO check return */
				if (minimalGui) {
					showImage(path);
				} else {
					printf("%s\n", path);
				}
			}
		}

		
		spBPQueueDestroy();

		free(cur_image_features);
	}

	


	if (tree) {
		destroyTree(tree);
	}
	if (numOfFeat) {
		free(numOfFeat);
	}
	if (features) {
		free(features);
	}
	spConfigDestroy(config);
}