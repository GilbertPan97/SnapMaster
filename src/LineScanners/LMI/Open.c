#include "GocatorCommon.h"

#include <GoSdk/GoSdk.h>
#include <stdio.h>
#include <stdlib.h>

kStatus Gocator_Open(Gocator_Handle* handle){
    if (handle->sensor != kNULL) {
        if(GoSystem_Start(handle->system) == kOK)
            clog("Info: Gocator sensor start successfully, laser on...");
        else 
            return GoSystem_Start(handle->system);
    } else {
        clog("Error: Gocator sensor start fail. No sensor in handle.");
        return kERROR;
    }

    kStatus status = kOK;
    // // enable sensor data channel
    // if ((status = GoSystem_EnableData(handle->system, kTRUE)) != kOK) {
    //     clog("Error: GoSensor_EnableData:%d\n", status);
    // }
    return status;
}

kStatus Gocator_Close(Gocator_Handle* handle){
    if (handle->sensor != kNULL) {
        if(GoSystem_Stop(handle->system) == kOK)
            clog("Info: Gocator sensor stop successfully, laser off...");
    } else {
        clog("Error: Gocator sensor stop fail. No sensor in handle.");
    }
}