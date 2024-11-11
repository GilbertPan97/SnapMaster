// logger.h
#ifndef LOGGER_H
#define LOGGER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*LogCallback)(const char* message);

void defaultLogCallback(const char* message);

void setLogCallback(LogCallback callback);

LogCallback getLogCallback();

void performTask();

#ifdef __cplusplus
}
#endif

#endif // LOGGER_H
