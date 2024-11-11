#ifndef CLOG_H
#define CLOG_H

#include "logger.h"

#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

// Buffer size for the log message
#define LOG_BUFFER_SIZE 256

// Custom log function
void clog(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif // CLOG_H
