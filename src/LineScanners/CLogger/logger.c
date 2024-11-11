// logger.c
#include "logger.h"
#include <stdio.h>

// Call back function pointer for logger
static LogCallback log_callback = defaultLogCallback;

void setLogCallback(LogCallback callback) {
    log_callback = callback;
}

LogCallback getLogCallback() {
    return log_callback;
}

// Default log function
void defaultLogCallback(const char* message) {
    printf("%s\n", message); // Print log messages to the console
}

void performTask() {
    if (log_callback) {
        log_callback("Task started.");
    }

    // Simulate some processing
    for (int i = 0; i < 5; ++i) {
        // Log progress
        char message[50];
        sprintf(message, "Processing item %d...", i);
        log_callback(message);
    }

    if (log_callback) {
        log_callback("Task completed.");
    }
}
