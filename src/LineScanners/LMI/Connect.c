#include "GocatorCommon.h"

#include <GoSdk/GoSdk.h>
#include <stdio.h>
#include <stdlib.h>

// Initializes the Gocator system and connects to the sensor.
kStatus Gocator_Connect(Gocator_Handle* handle, const char* sensorIp) {
    kStatus status;
    kIpAddress ipAddress;

    // Parse IP address into address data structure
    kIpAddress_Parse(&ipAddress, sensorIp);

    // obtain GoSensor object by sensor IP address
    if ((status = GoSystem_FindSensorByIpAddress(handle->system, &ipAddress, &handle->sensor)) != kOK) {
        clog("Error: GoSystem_FindSensor:%d\n", status);
        return status;
    }

    // create connection to GoSensor object
    if ((status = GoSensor_Connect(handle->sensor)) != kOK) {
        clog("Error: GoSensor_Connect:%d\n", status);
        return status;
    }

    // enable sensor data channel
    if ((status = GoSystem_EnableData(handle->system, kTRUE)) != kOK) {
        clog("Error: GoSensor_EnableData:%d\n", status);
        return status;
    }

    // retrieve setup handle
    if ((handle->setup = GoSensor_Setup(handle->sensor)) == kNULL) {
        clog("Error: GoSensor_Setup: Invalid Handle\n");
    }   

    return kOK;
}

// Initializes the Gocator system and connects to the sensor.
kStatus Gocator_DisConnect(Gocator_Handle* handle, ...) {
    kStatus status;
    va_list args;
    kIpAddress ipAddress;

    va_start(args, handle);
    const char* sensorIp = va_arg(args, const char*);
    va_end(args);

    // Check if sensorIp is provided (not NULL)
    if (sensorIp != NULL) {
        // Parse IP address into address data structure
        kIpAddress_Parse(&ipAddress, sensorIp);

        // Obtain GoSensor object by sensor IP address
        if ((status = GoSystem_FindSensorByIpAddress(handle->system, &ipAddress, &handle->sensor)) != kOK) {
            clog("Info: GoSystem_FindSensor: %s\n", sensorIp);
        }
    }

    // Disconnect GoSensor object if handle->sensor is valid
    if (handle->sensor != kNULL) {
        clog("Info: Found a connected sensor in handle, Disconnecting...");
        if ((status = GoSensor_Disconnect(handle->sensor)) != kOK) {
            clog("Error: GoSensor_Disconnect:%d\n", status);
            return status;
        }
        clog("Info: Disconnect finish.");
    } else {
        clog("Warning: No sensor found in handle. Disconnect fail, or ip address shoule be known.");
    }

    // Stop Gocator sensor system
    return kOK;
}


