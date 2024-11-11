#include "GocatorCommon.h"

#include <GoSdk/GoSdk.h>
#include <stdio.h>
#include <stdlib.h>

// Function to discover cameras
kStatus Gocator_Discover(Gocator_List* cameraList) {
    kStatus status;
    kAssembly api = kNULL;
    kAssembly apiLib = kNULL;
    GoSystem system = kNULL;

    // Initialize the API Library
    if ((status = kApiLib_Construct(&apiLib)) != kOK) {
        clog("Error: kApiLib_Construct:%d\n", status);
        return status;
    }

    // Construct Gocator API Library
    if ((status = GoSdk_Construct(&api)) != kOK) {
        clog("Error: GoSdk_Construct:%d\n", status);
        return status;
    }

    // Construct the GoSystem
    if ((status = GoSystem_Construct(&system, kNULL)) != kOK) {
        clog("Error: GoSystem_Construct:%d\n", status);
        return status;
    }

    // Allocate memory for camera list
    size_t cameraCount = GoSystem_SensorCount(system);
    cameraList->cam_info = (Gocator_Info*)malloc(cameraCount * sizeof(Gocator_Info));
    cameraList->count = 0;

    // Discover sensors
    if (cameraCount > 0){
        for (size_t i = 0; i < cameraCount; i++) {
            GoAddressInfo addressInfo;
            GoSensor sensor = GoSystem_SensorAt(system, i);

            if ((status = GoSensor_Address(sensor, &addressInfo)) == kOK) {
                Gocator_Info camera;
                camera.id = GoSensor_Id(sensor);
                kIpAddress_Format(addressInfo.address, camera.ipAddress, sizeof(camera.ipAddress));
                cameraList->cam_info[cameraList->count++] = camera;
                clog("Discovers sensor %d with IP %s\n", GoSensor_Id(sensor), camera.ipAddress);
            } else {
                clog("Error: Failed to get address for sensor ID: %d\n", GoSensor_Id(sensor));
            }
        }
    } else {
        clog("Warning: No devices found.");
    }

    // Clean up
    GoDestroy(system);
    GoDestroy(apiLib);
    GoDestroy(api);

    return kOK;
}

// Function to free the camera list
void Gocator_FreeList(Gocator_List* cameraList) {
    if (cameraList->cam_info) {
        free(cameraList->cam_info);
        cameraList->cam_info = NULL;
    }
}