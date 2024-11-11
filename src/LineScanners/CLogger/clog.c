#include "clog.h"

#include <stdio.h>
#include <stdarg.h>
#include <time.h>

// Custom log function
void clog(const char* format, ...) {
    LogCallback log_callback = getLogCallback();
    char logBuffer[LOG_BUFFER_SIZE]; // Define logBuffer within the function

    // Get current time
    time_t now = time(NULL);
    struct tm local;

    // Use localtime_s for thread-safe conversion
    localtime_s(&local, &now); // Use localtime_s instead of localtime

    char timestamp[20];
    snprintf(timestamp, sizeof(timestamp), "[%02d:%02d:%02d] ", local.tm_hour, local.tm_min, local.tm_sec);

    // Prepare the variable arguments
    va_list args;
    va_start(args, format);
    
    // Format the log message
    char logMessage[LOG_BUFFER_SIZE]; // Adjust size for timestamp
    vsnprintf(logMessage, sizeof(logMessage), format, args);
    
    va_end(args);
    
    // Concatenate timestamp and log message
    snprintf(logBuffer, sizeof(logBuffer), "%s%s", timestamp, logMessage);

    // Output the concatenated string (you can change this to store or send it elsewhere)
    log_callback(logMessage);
}