#include "GocatorCommon.h"

#include <stdio.h>
#include <stdlib.h>

// Receives profile data from the Gocator sensor.
kStatus Gocator_ReceiveProfileData(Gocator_Handle* handle, Gocator_Data* data) {
    unsigned int i, j, k, arrayIndex;
    GoDataMsg dataObj;
    GoDataSet dataset = kNULL;
    ProfilePoint* profileBuffer = NULL;
    kStatus status;
    k32u profilePointCount;
    GoStamp* stamp = kNULL;

    // enable sensor data channel
    if ((status = GoSystem_EnableData(handle->system, kTRUE)) != kOK) {
        clog("Error: GoSensor_EnableData:%d\n", status);
    }

    // retrieve total number of profile points prior to starting the sensor
    if (GoSetup_UniformSpacingEnabled(handle->setup)) {
        // Uniform spacing is enabled. The number is based on the X Spacing setting
        profilePointCount = GoSetup_XSpacingCount(handle->setup, GO_ROLE_MAIN);
    } else {
        // non-uniform spacing is enabled. The max number is based on the number of columns used in the camera. 
        profilePointCount = GoSetup_FrontCameraWidth(handle->setup, GO_ROLE_MAIN);
    }

    data->bufferSize = profilePointCount;
    profileBuffer = malloc(profilePointCount * sizeof(ProfilePoint));
    if (profileBuffer == kNULL) {
        return kERROR;          // Memory allocation failure
    }

    // Check if dataset is received
    GoSystem_ClearData(handle->system);         // Clear sensor dataset buffer
    if (GoSystem_ReceiveData(handle->system, &dataset, RECEIVE_TIMEOUT) != kOK) {
        clog("Data message received:\n"); 
        clog("Dataset count: %u\n", (k32u)GoDataSet_Count(dataset));
        return kERROR;          // No data received
    }

    // Initial data buffer
    data->profileBuffer = profileBuffer;
    data->pointCount = 0;

    // Loop through received data set
    for (i = 0; i < GoDataSet_Count(dataset); ++i) {
        dataObj = GoDataSet_At(dataset, i);
        
        // Handle Stamp messages
        if (GoDataMsg_Type(dataObj) == GO_DATA_MESSAGE_TYPE_STAMP) {
            GoStampMsg stampMsg = dataObj;
            clog("Stamp Message batch count: %u", (k32u)GoStampMsg_Count(stampMsg));
            
            for (j = 0; j < GoStampMsg_Count(stampMsg); ++j) {
                stamp = GoStampMsg_At(stampMsg, j);
                clog("  Timestamp: %llu", stamp->timestamp);
                clog("  Encoder: %lld", stamp->encoder);
                clog("  Frame index: %llu\n", stamp->frameIndex);
            }
        }

        // Handle Uniform Profile messages
        if (GoDataMsg_Type(dataObj) == GO_DATA_MESSAGE_TYPE_UNIFORM_PROFILE) {
            GoResampledProfileMsg profileMsg = dataObj;

            clog("Resampled Profile Message batch count: %u", (k32u)GoResampledProfileMsg_Count(profileMsg));

            for (k = 0; k < GoResampledProfileMsg_Count(profileMsg); ++k){
                short* rangeData = GoResampledProfileMsg_At(profileMsg, k); // Access the range data

                // Retrieve profile parameters
                double XResolution = NM_TO_MM(GoResampledProfileMsg_XResolution(profileMsg));
                double ZResolution = NM_TO_MM(GoResampledProfileMsg_ZResolution(profileMsg));
                double XOffset = UM_TO_MM(GoResampledProfileMsg_XOffset(profileMsg));
                double ZOffset = UM_TO_MM(GoResampledProfileMsg_ZOffset(profileMsg));

                for (arrayIndex = 0; arrayIndex < GoResampledProfileMsg_Width(profileMsg); ++arrayIndex) {
                    if (rangeData[arrayIndex] != INVALID_RANGE_16BIT) {
                        // Translate 16-bit range data to engineering units (mm)
                        profileBuffer[arrayIndex].x = XOffset + XResolution * arrayIndex; // Translate X
                        profileBuffer[arrayIndex].z = ZOffset + ZResolution * rangeData[arrayIndex]; // Translate Z
                        data->pointCount++;
                    } else {
                        // Mark invalid points
                        profileBuffer[arrayIndex].x = XOffset + XResolution * arrayIndex;
                        profileBuffer[arrayIndex].z = INVALID_RANGE_DOUBLE; // Indicate invalid range
                    }
                }
                clog("  Profile Valid Point %d out of max %d\n", data->pointCount++, profilePointCount);
            }

        }

        // Handle Profile Point Cloud messages
        if (GoDataMsg_Type(dataObj) == GO_DATA_MESSAGE_TYPE_PROFILE_POINT_CLOUD) {
            GoProfileMsg profileMsg = dataObj;

            clog("Profile Message batch count: %u", (k32u)GoProfileMsg_Count(profileMsg));

            for (k = 0; k < GoProfileMsg_Count(profileMsg); ++k){
                // Access the point cloud data
                kPoint16s* kData16s = GoProfileMsg_At(profileMsg, k);

                double XResolution = NM_TO_MM(GoProfileMsg_XResolution(profileMsg));
                double ZResolution = NM_TO_MM(GoProfileMsg_ZResolution(profileMsg));
                double XOffset = UM_TO_MM(GoProfileMsg_XOffset(profileMsg));
                double ZOffset = UM_TO_MM(GoProfileMsg_ZOffset(profileMsg));

                for (arrayIndex = 0; arrayIndex < GoProfileMsg_Width(profileMsg); ++arrayIndex) {
                    if (kData16s[arrayIndex].x != INVALID_RANGE_16BIT) {
                        profileBuffer[arrayIndex].x = XOffset + XResolution * kData16s[arrayIndex].x; // Translate X
                        profileBuffer[arrayIndex].z = ZOffset + ZResolution * kData16s[arrayIndex].y; // Translate Z
                        data->pointCount++;
                    } else {
                        // Mark invalid points
                        profileBuffer[arrayIndex].x = INVALID_RANGE_DOUBLE;
                        profileBuffer[arrayIndex].z = INVALID_RANGE_DOUBLE; // Indicate invalid range
                    }
                }
                clog("  Profile Valid Point %d out of max %d\n", data->pointCount++, profilePointCount);
            }
            
        }

        // Handle Profile Intensity messages
        if (GoDataMsg_Type(dataObj) == GO_DATA_MESSAGE_TYPE_PROFILE_INTENSITY) {
            GoProfileIntensityMsg intensityMsg = dataObj;
            clog("Intensity Message batch count: %u\n", (k32u)GoProfileIntensityMsg_Count(intensityMsg));

            for (k = 0; k < GoProfileIntensityMsg_Count(intensityMsg); ++k) {
                unsigned char* intensityData = GoProfileIntensityMsg_At(intensityMsg, k);
                for (arrayIndex = 0; arrayIndex < GoProfileIntensityMsg_Width(intensityMsg); ++arrayIndex) {
                    profileBuffer[arrayIndex].intensity = intensityData[arrayIndex]; // Store intensity values
                }
            }
        }
    }

    GoDestroy(dataset);
    return kOK;
}
