#include "GocatorCommon.h"

// Initializes the Gocator system and connects to the sensor.
kStatus Gocator_Initialize(Gocator_Handle* handle) {
    kStatus status;
    kIpAddress ipAddress;

    // construct Gocator API Library
    if ((status = GoSdk_Construct(&handle->api)) != kOK) {
        clog("Error: GoSdk_Construct:%d\n", status);
        return status;
    }

    // construct GoSystem object
    if ((status = GoSystem_Construct(&handle->system, kNULL)) != kOK) {
        clog("Error: GoSystem_Construct:%d\n", status);
        return status;
    }

    return kOK;
}

// Cleans up Gocator resources.
void Gocator_Cleanup(Gocator_Handle* handle) {
    if (handle->sensor) {
        GoSensor_Disconnect(handle->sensor);
        GoDestroy(handle->sensor);
    }
    if (handle->system) {
        GoSystem_Stop(handle->system);
        GoDestroy(handle->system);
    }
    GoDestroy(handle->api);
}