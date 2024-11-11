#include "GocatorCommon.h"
#include "LineScannerInterface.h"

#include <iostream>
#include <cstdlib>
#include <cstring>

LineScannerInterface::LineScannerInterface() : gocator() {

    kStatus status = Gocator_Initialize(&gocator);

    if (status != kOK){
        clog("Error: Sensor initial fail.");
    }
}

LineScannerInterface::~LineScannerInterface() {
    SetStatus(false);
    Shutdown();
}

CameraStatus LineScannerInterface::Scan(std::vector<CameraInfo>& cameraList) {
    Gocator_List cList;         // C data type
    cList.cam_info = nullptr;
    cList.count = 0;

    kStatus status = Gocator_Discover(&cList); // Call to the external camera discovery function

    if (status != kOK) {
        return CameraStatus::Go_ERROR;
    }

    // Populate the output vector with discovered cameras
    cameraList = ConvertToCameraInfoList(cList);

    Gocator_FreeList(&cList); // Free the allocated memory for the camera list

    return CameraStatus::Go_OK;
}

CameraStatus LineScannerInterface::Connect(const std::string& cameraIp) {
    const char* ip_s = _strdup(cameraIp.c_str());
    kStatus status = Gocator_Connect(&gocator, ip_s);

    if (status == kOK){
        return CameraStatus::Go_OK;
    } else
        return CameraStatus::Go_NOT_CONNECTED;
}

CameraStatus LineScannerInterface::GrabOnce() {
    kStatus status;

    if(Gocator_ReceiveProfileData(&gocator, &data) != kOK){
        clog("Error: ReceiveProfileData fail, skip single grab.");
        return CameraStatus::Go_ERROR;
    };

    RemoveInvalidPoints();
    return CameraStatus::Go_OK;
}

CameraStatus LineScannerInterface::SetStatus(bool open) {
    kStatus status;
    if (open){
        status = Gocator_Open(&gocator);
    } else {
        status = Gocator_Close(&gocator);
    }

    if (status = kOK){
        clog("Info: Set status successfully (status: %s).\n", open ? "true" : "false");
        return CameraStatus::Go_OK;
    } else {

    }
        
}

CameraStatus LineScannerInterface::Disconnect() {
    kStatus status = Gocator_DisConnect(&gocator);

    if (status == kOK){
        return CameraStatus::Go_OK;
    } else
        return CameraStatus::Go_ERROR;
}

CameraStatus LineScannerInterface::Disconnect(const std::string& cameraIp) {
    const char* ip_s = _strdup(cameraIp.c_str());
    kStatus status = Gocator_DisConnect(&gocator, ip_s);

    if (status == kOK){
        return CameraStatus::Go_NOT_CONNECTED;
    } else
        return CameraStatus::Go_ERROR;
}

Gocator_Data LineScannerInterface::RetriveData() {
    if (data.pointCount > 0){
        clog("Info: Valid points - %d", data.pointCount);
        return data;
    } else {
        clog("Error: No sensor data can be retrive.");
        return data;
    }
}

void LineScannerInterface::Shutdown() {
    // Shutdown the camera and free resources
    if (gocator.api != NULL) {
        Gocator_Cleanup(&gocator);
        clog("Device closed down.");
    }
}

// Function to convert GoSdk_CameraList to std::vector<CameraInfo>
std::vector<CameraInfo> LineScannerInterface::ConvertToCameraInfoList(const Gocator_List& sdkCameraList) {
    std::vector<CameraInfo> cameraInfoList;

    // Iterate through the GoSdk_CameraList
    for (size_t i = 0; i < sdkCameraList.count; ++i) {
        CameraInfo cameraInfo;
        cameraInfo.id = sdkCameraList.cam_info[i].id; // Copy the ID
        cameraInfo.ipAddress = sdkCameraList.cam_info[i].ipAddress; // Copy the IP address

        // Add the cameraInfo to the vector
        cameraInfoList.push_back(cameraInfo);
    }

    return cameraInfoList; // Return the populated vector
}

void LineScannerInterface::RemoveInvalidPoints() {
    // Create a temporary container to store valid ProfilePoint
    std::vector<ProfilePoint> validPoints;

    // Iterate through profileBuffer and filter out invalid points
    for (size_t i = 0; i < data.bufferSize; ++i) {
        // std::cout << "Point-[ " << i << " ]: x = " << data.profileBuffer[i].x
        //           << ", y = [ " << data.profileBuffer[i].z << " ]\n";
        // Check if both x and z are not equal to INVALID_RANGE_DOUBLE
        if (data.profileBuffer[i].x != INVALID_RANGE_DOUBLE && 
            data.profileBuffer[i].z != INVALID_RANGE_DOUBLE) {
            validPoints.push_back(data.profileBuffer[i]); // Add valid point to the temporary container
        }
    }

    // Update pointCount to the number of valid points
    data.pointCount = validPoints.size();

    // Reallocate profileBuffer to store valid points
    if (data.pointCount > 0) {
        // Allocate memory for the valid points
        data.profileBuffer = new ProfilePoint[data.pointCount];
        std::copy(validPoints.begin(), validPoints.end(), data.profileBuffer); // Copy valid points to profileBuffer
    } else {
        // If no valid points, set profileBuffer to nullptr
        delete[] data.profileBuffer; // Free existing memory
        data.profileBuffer = nullptr;
    }
}

