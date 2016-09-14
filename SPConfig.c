#include "SPConfig.h"

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#define MAX_LINE_LEN  (1024 + 1) /* max len of a line in the config file, plus null */
#define MAX_SYSTEM_VARIABLE_STR_LEN (1024 + 1) /* max str var len, plus null */
#define MAX_INT_STR_LEN (11 + 1) /* max size of decimal string int plus null */

#define FEATS_FILE_SUFFIX ".feats" /* the suffix of features file */

/* a union for the data of system variable, allows convinant access */
union system_variabale_data {
	char string_value[MAX_SYSTEM_VARIABLE_STR_LEN];
    int int_value;
    bool bool_value;
};

/* a function pointer type for the constraint */
typedef bool(*constraint_t)(union system_variabale_data *);

/* types of system variables (bool and enum are not real types) */
enum variable_type_t {
	VAR_TYPE_INT,
	VAR_TYPE_STRING
};

/* a description of a system variable */
struct system_variable_description {
    char * name;
    enum variable_type_t type;
    bool has_defualt_value;
    union system_variabale_data default_value;
    constraint_t constraint;
};

/* describes a state of a system variable in a specific config */
struct system_variable_state {
    bool is_initialized;
    union system_variabale_data data;
};

/* ------ now follow a few constraint definitions, they are very short and self explanatory --------------*/

bool no_spaces_constraint(union system_variabale_data * data) {
    char * dir = data->string_value;
    while(*dir) {
        if (isspace(*dir)) return false;
        dir++;
    }
    return true;
}

bool suffix_constraint(union system_variabale_data * data) {
    return strcmp(data->string_value, ".jpg") == 0 ||
           strcmp(data->string_value, ".png") == 0 ||
           strcmp(data->string_value, ".bmp") == 0 ||
           strcmp(data->string_value, ".gif") == 0 ;
}

bool positive_int_constraint(union system_variabale_data * data) {
    return data->int_value > 0;
}

bool PCA_dim_constraint(union system_variabale_data * data) {
    return data->int_value >= 10 && data->int_value <= 28;
}

bool bool_constraint(union system_variabale_data * data) {
    return strcmp(data->string_value, "true") == 0 ||
           strcmp(data->string_value, "false") == 0 ;
}

bool split_method_constraint(union system_variabale_data * data) {
    return strcmp(data->string_value, "RANDOM") == 0 ||
           strcmp(data->string_value, "MAX_SPREAD") == 0 ||
           strcmp(data->string_value, "INCREMENTAL") == 0 ;
}

bool logger_level_constraint(union system_variabale_data * data) {
    return data->int_value >= 1 && data->int_value <= 4;
}

/* -------------------------------------------------------------------------------------------------------*/

#define NO_DEFAULT_VALUE {{0}}

/* This is global because it's not specific to a single configuration */
/* This is an array with the descriptions of all system variables (name, type, default value, constraint) */
static const struct system_variable_description ALL_SYSTEM_VARIABLES[] = {
    {"spImagesDirectory", VAR_TYPE_STRING, false, NO_DEFAULT_VALUE, no_spaces_constraint},
    {"spImagesPrefix", VAR_TYPE_STRING, false, NO_DEFAULT_VALUE, no_spaces_constraint},
    {"spImagesSuffix", VAR_TYPE_STRING, false, NO_DEFAULT_VALUE, suffix_constraint},
    {"spNumOfImages", VAR_TYPE_INT, false, NO_DEFAULT_VALUE, positive_int_constraint},
    {"spPCADimension", VAR_TYPE_INT, true, {{20}}, PCA_dim_constraint},
	{"spPCAFilename", VAR_TYPE_STRING, true, {"pca.yml"}, no_spaces_constraint },
    {"spNumOfFeatures", VAR_TYPE_INT, true, {{100}}, positive_int_constraint},
	{"spExtractionMode", VAR_TYPE_STRING, true, {{true}}, bool_constraint }, /*bool is not really a type, only a string with a bool constraint*/
    {"spNumOfSimilarImages", VAR_TYPE_INT, true, {{1}}, positive_int_constraint},
	{"spKDTreeSplitMethod", VAR_TYPE_STRING, true, {"MAX_SPREAD"}, split_method_constraint }, /*enum is not really a type*/
    {"spKNN", VAR_TYPE_INT, true, {{1}}, positive_int_constraint},
    {"spMinimalGUI", VAR_TYPE_STRING, true, {{false}}, bool_constraint},
    {"spLoggerLevel", VAR_TYPE_INT, true, {{3}}, logger_level_constraint},
    {"spLoggerFilename", VAR_TYPE_STRING, true, {"stdout"}, no_spaces_constraint},
};

#define NUM_OF_SYSTEM_VARIABLES (sizeof(ALL_SYSTEM_VARIABLES) / sizeof(ALL_SYSTEM_VARIABLES)[0])

/* The state of the config */
struct sp_config_t {
	struct system_variable_state vars[NUM_OF_SYSTEM_VARIABLES];
};


struct system_variable_state * spConfigRetrieveSystemVariable(const SPConfig config, char * name) {
	unsigned int i;
    for (i = 0; i < NUM_OF_SYSTEM_VARIABLES; i++) {
        if (strcmp(ALL_SYSTEM_VARIABLES[i].name, name) == 0) {
            return &config->vars[i];
        }
    }
	return NULL;
}

/* Inner function - returns the value of the var named name,
 * or the default if it was not set by the config */
static const union system_variabale_data * getVarOrDefaultValue(const SPConfig config, char * name) {
	unsigned int i;
	for (i = 0; i < NUM_OF_SYSTEM_VARIABLES; i++) {
		if (strcmp(ALL_SYSTEM_VARIABLES[i].name, name) == 0) {
			if (config->vars[i].is_initialized) {
				return &config->vars[i].data;
			}
			else if (ALL_SYSTEM_VARIABLES[i].has_defualt_value){
				return &ALL_SYSTEM_VARIABLES[i].default_value;
			}
			else {
				return NULL;
			}
		}
	}
	return NULL;
}

/* Inner function - checks if str contains a valid int */
static bool is_valid_int (char * str) {
    if (str == NULL) return false;
    if (str[0] == '-') str++;
    if (str[0] == '\0') return false;
    while (*str) {
        if (!isdigit(*str)) return false;
        str++;
    }
    return true;
}

/* Inner function - processes a newly read var value in the config,
 * if the value is valid, the system variables array will be updated */
static SP_CONFIG_MSG spConfigProcessVar(const SPConfig config, size_t system_var_index, char * variable_value, size_t value_length) {
    /* if this is an int */
    if (ALL_SYSTEM_VARIABLES[system_var_index].type == VAR_TYPE_INT) {
        char buf[MAX_INT_STR_LEN+1]; /* temp buffer to hold just the number */
        
        /* first we validate the input */
        
        if (value_length > MAX_INT_STR_LEN) {
            return SP_CONFIG_INVALID_INTEGER;
        }
        
        memcpy(buf, variable_value, value_length);
        buf[value_length] = '\0';
        
        if (!is_valid_int(buf)) {
            return SP_CONFIG_INVALID_INTEGER;
        }
        
        /* now we convert the string to int */
		config->vars[system_var_index].data.int_value = atoi(buf);
        
    } else if (ALL_SYSTEM_VARIABLES[system_var_index].type == VAR_TYPE_STRING) {
        
        /* make sure the string is not empty */
        if (value_length == 0) {
            return SP_CONFIG_INVALID_STRING;
        }
        
        /* now copy it to the state */
		memcpy(config->vars[system_var_index].data.string_value, variable_value, value_length);
		config->vars[system_var_index].data.string_value[value_length] = '\0';
    } else {
        /* we shouldn't get here, there should be no other types */
        assert(false);
    }
    
    /* check the constraint */
	if (!ALL_SYSTEM_VARIABLES[system_var_index].constraint(&config->vars[system_var_index].data)) {
        return SP_CONFIG_CONSTRAINT_NOT_MET;
    }
        
    /* processed successfuly */
	config->vars[system_var_index].is_initialized = true;
    
    return SP_CONFIG_SUCCESS;
}

/* Inner function - processes a newly read line in the config,
 * if the line contains a system variable setting (not an empty line
 * or a comment) - the config state will be updated. */
static SP_CONFIG_MSG spConfigProcessLine(const SPConfig config, char * line) {
    char * variable_name = NULL;
    size_t varibe_name_length = 0;
    char * variable_value = NULL;
    size_t varibe_value_length = 0;
    unsigned int i = 0;

	assert(line != NULL && config != NULL);
    
    while (isspace(*line)) line++; /* skip whitespaces */
    if (*line == '#' || *line == '\0') return SP_CONFIG_SUCCESS; /* ignore comments and empty lines */
    
    variable_name = line;
    
    /* read variable name */
    while (*line != '=' && *line != '\0' && !isspace(*line)) {
        line++;
        varibe_name_length++;
    }
    
    /* get to the start of the value */
    while (isspace(*line)) line++;
    if (*line != '=') return SP_CONFIG_INVALID_LINE;
    line++;
    while (isspace(*line)) line++;
    
    /* make sure there are no spaces in value */
    variable_value = line;
    while (*line != '\0' && !isspace(*line)) {
        line++;
        varibe_value_length++;
    }
    while (isspace(*line)) line++; /* allow trailing whitespaces */
    if (*line != '\0') return SP_CONFIG_INVALID_LINE;
    
    for (i = 0; i < NUM_OF_SYSTEM_VARIABLES; i++) {
        /* if this a valid name of one of the variables */
        if (memcmp(variable_name, ALL_SYSTEM_VARIABLES[i].name, varibe_name_length) == 0) {
			return spConfigProcessVar(config, i, variable_value, varibe_value_length);
        }
    }
    /* if the config line doesn't match any
       known variable - it's an error */
    return SP_CONFIG_INVALID_LINE;
}

/* For documentation see header file */
SPConfig spConfigCreate(const char* filename, SP_CONFIG_MSG* msg) {
	FILE * file = NULL;
	struct sp_config_t * sp_config_state = NULL;
	char line_buf[MAX_LINE_LEN];
	unsigned int i = 0, line = 0;

	if (filename == NULL) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return NULL;
	}


	/* open the file */
	file = fopen(filename, "r");
	if (file == NULL) {
		*msg = SP_CONFIG_CANNOT_OPEN_FILE;
		return NULL;
	}

	/* allocate config state */
	sp_config_state = (struct sp_config_t *)malloc(sizeof(struct sp_config_t));
	if (sp_config_state == NULL) {
		*msg = SP_CONFIG_ALLOC_FAIL;
		return NULL;
	}
	memset(sp_config_state, 0, sizeof(struct sp_config_t));

	*msg = SP_CONFIG_SUCCESS;

	/* process all lines */
	while (fgets(line_buf, MAX_LINE_LEN, file) != NULL) {
		*msg = spConfigProcessLine(sp_config_state, line_buf);
		if (*msg != SP_CONFIG_SUCCESS) {
			if (*msg == SP_CONFIG_INVALID_LINE || *msg == SP_CONFIG_INVALID_INTEGER || *msg == SP_CONFIG_INVALID_STRING) {
				printf("File: %s\nLine: %d\nMessage: Invalid configuration line\n", filename, line+1);
			}
			else if (*msg == SP_CONFIG_CONSTRAINT_NOT_MET) {
				printf("File: %s\nLine: %d\nMessage: Invalid value - constraint not met\n", filename, line+1);
			}
			break;
		}
		line++;
	}

	/* make sure that all variables that don't have default values have been loaded */
	if (*msg == SP_CONFIG_SUCCESS) {
		/* then we check all fields together in a more generic way */
		for (i = 0; i < NUM_OF_SYSTEM_VARIABLES; i++) {
			if (!ALL_SYSTEM_VARIABLES[i].has_defualt_value && !sp_config_state->vars[i].is_initialized) {
				*msg = SP_CONFIG_MISSING_NON_DEFAULT_VALUE;

				printf("File: %s\nLine: %d\nMessage: Parameter %s is not set\n", filename, line, ALL_SYSTEM_VARIABLES[i].name);
				break;
			}
		}
	}

	fclose(file);

	if (*msg != SP_CONFIG_SUCCESS) {
		free(sp_config_state);
		sp_config_state = NULL;
	}

	return sp_config_state;
}

/*
* Returns true if the field var_name has the value of true, false otherwise.
*
* @param var_name - the name of the system variable
* @param config - the configuration structure
* @assert msg != NULL
* @param msg - pointer in which the msg returned by the function is stored
* @return true if spExtractionMode = true, false otherwise.
*
* - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
* - SP_CONFIG_SUCCESS - in case of success
*/
static bool getBoolVal(char * var_name, const SPConfig config, SP_CONFIG_MSG* msg) {
	const union system_variabale_data * var = NULL;
	assert(msg != NULL && var_name != NULL);

	if (!config) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return false;
	}

	var = getVarOrDefaultValue(config, var_name);
	assert(var != NULL);

	*msg = SP_CONFIG_SUCCESS;

	return var->string_value[0] == 't'; /* t for true, f for false */
										/* the bool constraint assures us that this check is OK */
}

/*
* Returns the value of the system variable var_name, which is a positive int.
*
* @param var_name - the name of the requested system variable
* @param config - the configuration structure
* @assert msg != NULL
* @param msg - pointer in which the msg returned by the function is stored
* @return positive integer in success, negative integer otherwise.
*
* - SP_CONFIG_INVALID_ARGUMENT - if config == NULL
* - SP_CONFIG_SUCCESS - in case of success
*/
static int getIntVal(char * var_name, const SPConfig config, SP_CONFIG_MSG* msg) {
	const union system_variabale_data * var = NULL;
	assert(msg != NULL && var_name != NULL);

	if (!config) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return -1;
	}

	var = getVarOrDefaultValue(config, var_name);
	assert(var != NULL);

	*msg = SP_CONFIG_SUCCESS;
	return var->int_value;
}

/**
* The function stores in dest the value of the field named var_name.
*
* the address given by dest must contain enough space to store the string.
*
* @param var_name - the name of the string field
* @param dest - an address to store the result in, it must contain enough space.
* @param config - the configuration structure
* @return
*  - SP_CONFIG_INVALID_ARGUMENT - if imagePath == NULL or config == NULL
*  - SP_CONFIG_SUCCESS - in case of success
*/
static SP_CONFIG_MSG getStringVal(char * var_name, char* dest, const SPConfig config) {
	const union system_variabale_data * var = NULL;
	if (!dest || !config) {
		return SP_CONFIG_INVALID_ARGUMENT;
	}

	var = getVarOrDefaultValue(config, var_name);
	assert(var); /*this function should only be called if the create succeeded*/

	strcpy(dest, var->string_value);

	return SP_CONFIG_SUCCESS;
}

bool spConfigIsExtractionMode(const SPConfig config, SP_CONFIG_MSG* msg) {
	return getBoolVal("spExtractionMode", config, msg);
}

bool spConfigMinimalGui(const SPConfig config, SP_CONFIG_MSG* msg) {
	return getBoolVal("spMinimalGUI", config, msg);
}

int spConfigGetNumOfImages(const SPConfig config, SP_CONFIG_MSG* msg){
	return getIntVal("spNumOfImages", config, msg);
}

int spConfigGetNumOfFeatures(const SPConfig config, SP_CONFIG_MSG* msg){
	return getIntVal("spNumOfFeatures", config, msg);
}

int spConfigGetPCADim(const SPConfig config, SP_CONFIG_MSG* msg) {
	return getIntVal("spPCADimension", config, msg);
}

int spConfigGetNumOfSimilarImages(const SPConfig config, SP_CONFIG_MSG* msg) {
	return getIntVal("spNumOfSimilarImages", config, msg);
}

int spConfigGetKNN(const SPConfig config, SP_CONFIG_MSG* msg) {
	return getIntVal("spKNN", config, msg);
}

int spConfigGetLoggerLevel(const SPConfig config, SP_CONFIG_MSG* msg) {
	return getIntVal("spLoggerLevel", config, msg);
}

KDTREE_SPLIT_METHOD spConfigGetKDTreeSplitMethod(const SPConfig config, SP_CONFIG_MSG* msg) {
	const union system_variabale_data * method = NULL;
	assert(msg != NULL);
	if (!config) {
		*msg = SP_CONFIG_INVALID_ARGUMENT;
		return -1;
	}

	method = getVarOrDefaultValue(config, "spKDTreeSplitMethod");
	assert(method != NULL);

	*msg = SP_CONFIG_SUCCESS;

	if (strcmp(method->string_value, "MAX_SPREAD") == 0) {
		return MAX_SPREAD;
	} else if (strcmp(method->string_value, "RANDOM") == 0) {
		return RANDOM;
	} else if (strcmp(method->string_value, "INCREMENTAL") == 0) {
		return INCREMENTAL;
	} else {
		assert(false); /* because of the constraint, we should never get here */
	}

	return -1;
}

SP_CONFIG_MSG spConfigGetImagePath(char* imagePath, const SPConfig config,
	int index) {
	const union system_variabale_data * numOfImages = NULL, *path = NULL, *prefix = NULL, *suffix = NULL;
	if (!imagePath || !config) {
		return SP_CONFIG_INVALID_ARGUMENT;
	}

	numOfImages = getVarOrDefaultValue(config, "spNumOfImages");
	path = getVarOrDefaultValue(config, "spImagesDirectory");
	prefix = getVarOrDefaultValue(config, "spImagesPrefix");
	suffix = getVarOrDefaultValue(config, "spImagesSuffix");
	assert(numOfImages && path && prefix && suffix);
	
	if (index >= numOfImages->int_value) {
		return SP_CONFIG_INDEX_OUT_OF_RANGE;
	}

	sprintf(imagePath, "%s%s%d%s", path->string_value, prefix->string_value, index, suffix->string_value);

	return SP_CONFIG_SUCCESS;
}

SP_CONFIG_MSG spConfigGetFeatsPath(char* featsPath, const SPConfig config,
	int index) {
	const union system_variabale_data * numOfImages = NULL, *path = NULL, *prefix = NULL;
    char * suffix = FEATS_FILE_SUFFIX;
	if (!featsPath || !config) {
		return SP_CONFIG_INVALID_ARGUMENT;
	}

	numOfImages = getVarOrDefaultValue(config, "spNumOfImages");
	path = getVarOrDefaultValue(config, "spImagesDirectory");
	prefix = getVarOrDefaultValue(config, "spImagesPrefix");
	assert(numOfImages && path && prefix && suffix);
	
	if (index >= numOfImages->int_value) {
		return SP_CONFIG_INDEX_OUT_OF_RANGE;
	}

	sprintf(featsPath, "%s%s%d%s", path->string_value, prefix->string_value, index, suffix);

	return SP_CONFIG_SUCCESS;
}

SP_CONFIG_MSG spConfigGetPCAPath(char* pcaPath, const SPConfig config) {
	const union system_variabale_data * var = NULL;
	if (!pcaPath || !config) {
		return SP_CONFIG_INVALID_ARGUMENT;
	}

	var = getVarOrDefaultValue(config, "spImagesDirectory");
	assert(var); /*this function should only be called if the create succeeded*/
	strcpy(pcaPath, var->string_value);

	var = getVarOrDefaultValue(config, "spPCAFilename");
	assert(var);
	strcat(pcaPath, var->string_value);
	
	return SP_CONFIG_SUCCESS;
}

SP_CONFIG_MSG spConfigGetLoggerFilename(char* loggerFilename, const SPConfig config) {
	return getStringVal("spLoggerFilename", loggerFilename, config);
}

void spConfigDestroy(SPConfig config) {
	if (config) {
		free(config);
	}
}
