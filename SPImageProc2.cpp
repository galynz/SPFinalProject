#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>
#include "SPImageProc2.h"
extern "C" {
#include "SPLogger.h"
#include "FeaturesStorage.h"
}

#if defined ( WIN32 )
#define __func__ __FUNCTION__
#endif



#define PCA_MEAN_STR "mean"
#define PCA_EIGEN_VEC_STR "e_vectors"
#define PCA_EIGEN_VAL_STR "e_values"
#define STRING_LENGTH 1024
#define WARNING_MSG_LENGTH 2048

#define GENERAL_ERROR_MSG "An error occurred"
#define PCA_DIM_ERROR_MSG "PCA dimension couldn't be resolved"
#define PCA_FILE_NOT_EXIST "PCA file doesn't exist"
#define PCA_FILE_NOT_RESOLVED "PCA filename couldn't be resolved"
#define NUM_OF_IMAGES_ERROR "Number of images couldn't be resolved"
#define NUM_OF_FEATS_ERROR "Number of features couldn't be resolved"
#define MINIMAL_GUI_ERROR "Minimal GUI mode couldn't be resolved"
#define IMAGE_PATH_ERROR "Image path couldn't be resolved"
#define IMAGE_NOT_EXIST_MSG ": Images doesn't exist"
#define MINIMAL_GUI_NOT_SET_WARNING "Cannot display images in non-Minimal-GUI mode"
#define ALLOC_ERROR_MSG "Allocation error"
#define INVALID_ARG_ERROR "Invalid arguments"


sp::ImageProc::ImageProc(const SPConfig config) {
	config;
}

SPPoint* sp::ImageProc::getImageFeatures(const char* imagePath, int index,
		int* numOfFeats) {
            char buf[1024];
            int len = strlen(imagePath);
            strcpy(buf, imagePath);
            buf[len+2] = '\0';
            buf[len+1] = 'x';
			buf[len] = 't';
				buf[len - 1] = 'a';
			buf[len - 2] = 'e';
            buf[len-3] = 'f';
            return readImageFeatures(buf, numOfFeats);
	
}

void sp::ImageProc::showImage(const char* imgPath) {
    imgPath;
}

