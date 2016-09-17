#include "FeaturesStorage.h"
#include "common.h"

#include <stdio.h>
#include <assert.h>
#include <malloc.h>

SPPoint* readImageFeatures(const char* featsPath, int* numOfFeats) {
	FILE * file = NULL;
	SPPoint * features = NULL;
	int file_size = 0;
	char * all_data = NULL;
	char * data_end = NULL;
	char * cur_place = NULL;
	int i = 0;
	bool success = false;

	assert(featsPath != NULL && numOfFeats != NULL);

	file = fopen(featsPath, "rb");
	VALIDATE_WITH_LOG(file, "Could not open feature file");
	
	VALIDATE_WITH_LOG(0 == fseek(file, 0, SEEK_END) && -1 != (file_size = ftell(file))
		&& 0 != fseek(file, 0, SEEK_SET), "Error getting file size");

	VALIDATE_WITH_LOG(fread(numOfFeats, sizeof(int), 1, file) == 1, "Error reading num of features");

	file_size -= sizeof(*numOfFeats); /* from now on we only need to read the features */

	all_data = (char *)malloc(file_size);
	features = (SPPoint *)malloc((*numOfFeats) * sizeof(SPPoint));
	VALIDATE_WITH_LOG(all_data && features, "Allocation error");

	VALIDATE_WITH_LOG((int)fread(all_data, 1, file_size, file) == file_size, "Error reading feature data");

	cur_place = all_data;
	data_end = all_data + file_size;
	while (cur_place != data_end) {
		features[i++] = spPointDeserialize(cur_place, &cur_place);
		if (!features[i]) {
			LOG_ERROR("Error deserializing some point");
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
	bool success = false;

	assert(featsPath != NULL && features != NULL);

	file = fopen(featsPath, "wb");
	VALIDATE_WITH_LOG(file, "Could not open feature file");

	VALIDATE_WITH_LOG(fwrite(&numOfFeats, sizeof(int), 1, file) == 1, "Error writing num of features to file");

	for (i = 0; i < numOfFeats; i++) {
		data_size = spPointSerialize(features[i], &point_data);
		VALIDATE_WITH_LOG(data_size != -1, "Error serializing some point");

		VALIDATE_WITH_LOG((int)fwrite(point_data, 1, data_size, file) == data_size, "Error writing some point data to the file");

		free(point_data);
		point_data = NULL;
	}

	success = true;

cleanup:
	if (point_data) free(point_data);
	if (file) fclose(file);
	return success;
}