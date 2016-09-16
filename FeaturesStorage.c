#include "FeaturesStorage.h"

#include <stdio.h>
#include <assert.h>

SPPoint* readImageFeatures(const char* featsPath, int* numOfFeats) {
	FILE * file = NULL;
	SPPoint * features = NULL;
	size_t file_size = 0;
	char * all_data = NULL;
	char * data_end = NULL;
	int i = 0;
	bool success = true;

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
		goto cleanup;
	}

	if (fread(numOfFeats, sizeof(int), 1, file) != sizeof(int)) {
		/* TODO log */
		goto cleanup;
	}

	file_size -= sizeof(*numOfFeats); /* from now on we only need to read the features */

	all_data = (SPPoint *)malloc(file_size);
	if (!all_data) {
		goto cleanup;
	}

	features = (SPPoint *)malloc((*numOfFeats) * sizeof(SPPoint));
	if (!features) {
		goto cleanup;
	}

	if (fread(all_data, 1, file_size, file) != file_size) {
		/* TODO log */
		goto cleanup;
	}

	data_end = all_data + file_size;
	while (all_data != data_end) {
		features[i++] = spPointDeserialize(all_data, &all_data);
		if (!features[i]) {
			/* TODO log */
			while (i--) {
				spPointDestroy(features[i]);
			}
			goto cleanup;
		}
	}
	
	success = true;

cleanup:
	if (!success) {
		if (features) free(features);
		features = NULL;
	}
	if (all_data) free(all_data);
	if (file) fclose(file);
	return features;
}

bool writeImageFeatures(const char* featsPath, int numOfFeats, SPPoint* features) {
	FILE * file = NULL;
	int i = 0;
	char * point_data = NULL;
	int data_size = 0;

	assert(featsPath != NULL && features != NULL);

	file = fopen(featsPath, "wb");
	if (!file) {
		/* TODO log */
		return false;
	}

	if (fwrite(&numOfFeats, sizeof(int), 1, file) != sizeof(int)) {
		/* TODO log */
		fclose(file);
		return false;
	}

	for (i = 0; i < numOfFeats; i++) {
		data_size = spPointSerialize(features[i], &point_data);
		if (data_size == -1) {
			/* TODO log */
			fclose(file);
			return false;
		}

		if (fwrite(point_data, 1, data_size, file) != data_size) {
			/* TODO log */
			fclose(file);
			free(point_data);
			return false;
		}

		free(point_data);
	}

	fclose(file);
	return true;
}