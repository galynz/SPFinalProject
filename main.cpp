#include "SPConfig.h"

#define MAIN_RETURN_FAILURE (-1)
#define MAIN_RETURN_SUCCESS (0)

#define DEFAULT_CONFIG ("spcbir.config")

/* helper macro */
#define VALIDATE_SUCCESS_OR_RETURN(msg) if (msg != SP_CONFIG_SUCCESS) return MAIN_RETURN_FAILURE;

int main(int argc, char * argv[]) {
	const char * config_filename = DEFAULT_CONFIG;
	SP_CONFIG_MSG res_msg = SP_CONFIG_SUCCESS;
	SPConfig config = NULL;

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

	if (spConfigIsExtractionMode(config, res_msg)) {

	}
	VALIDATE_SUCCESS_OR_RETURN(res_msg);

	spConfigDestroy(config);
}