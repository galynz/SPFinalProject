#include "FeaturesStorage.h"

#include <stdio.h>

SPPoint* readImageFeatures(const char* featsPath, int* numOfFeats) {
	FILE * file = NULL;
	SPPoint * features = NULL;
	size_t file_size = 0;

	assert(featsPath != NULL && numOfFeats != NULL);

	file = fopen(featsPath, "rb");
	if (!file) {
		/* TODO log */
		return NULL;
	}

	if (0 != fseek(file, 0, SEEK_END) ||
		-1 == (file_size = ftell(file)) ||
		0 != fseek(file, 0, SEEK_SET)) {
		/*TODO log could not get file size*/
		fclose(file);
		return NULL;
	}

	if (fread(numOfFeats, sizeof(int), 1, file) != 1) {
		/* TODO log */
		fclose(file);
		return NULL;
	}

	file_size -= sizeof(*numOfFeats); /* from now on we only need to read the features */

	features = (SPPoint *)malloc(file_size);
	if (!features) {
		fclose(file);
		return NULL;
	}

	if (fread(features, 1, file_size, file) != file_size) {
		/* TODO log */
		free(features);
		features = NULL;
	}

	fclose(file);
	return features;
}