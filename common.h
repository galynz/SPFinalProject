#ifndef COMMON_H_
#define COMMON_H_

#include "SPLogger.h"

typedef enum {
    MAX_SPREAD,
    RANDOM,
    INCREMENTAL
} KDTREE_SPLIT_METHOD;

#if defined ( WIN32 )
#define __func__ __FUNCTION__
#endif

/* helper macros */
#define LOG_ERROR(msg) spLoggerPrintError(msg, __FILE__, __func__, __LINE__)
#define VALIDATE_WITH_LOG(cond, msg) do {if(!(cond)) {LOG_ERROR(msg); goto cleanup;}} while(0)

#define MAX_FILE_PATH (1024 + 1) /* 1024 chars + 1 null terminator */

#endif /* COMMON_H_ */
