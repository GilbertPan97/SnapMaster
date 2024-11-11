#ifndef LINE_SCANNER_INTERFACE_H
#define LINE_SCANNER_INTERFACE_H

#include "GocatorCommon.h"

#include <string>
#include <vector>
#include <memory>

// Define camera status enumeration
enum class CameraStatus {
    Go_OK,
    Go_ERROR,
    Go_NOT_CONNECTED,
};

// Struct to hold camera information
struct CameraInfo {
    int id;
    std::string brand;          // Camera brand
    std::string ipAddress;      // IP address
    bool isConnected;           // Connection status, true if connected, false if not

    // Default constructor
    CameraInfo() 
        : id(-1), brand("N/A"), ipAddress("0.0.0.0"), isConnected(false) {} // Initialize with default values

    // Parameterized constructor
    CameraInfo(int cameraId, const std::string& cameraBrand, const std::string& ip)
        : id(cameraId), brand(cameraBrand), ipAddress(ip), isConnected(false) {} // Default isConnected to false
};

// Define the Line Scanner Interface class
class LineScannerInterface {

public:
    LineScannerInterface();
    
    ~LineScannerInterface();

    // Scan available cameras
    CameraStatus Scan(std::vector<CameraInfo>& cameraList);

    // Connect the camera with the given IP address
    CameraStatus Connect(const std::string& cameraIp);

    // Start the camera grab
    CameraStatus GrabOnce();

    // Open and close sensor laser
    CameraStatus SetStatus(bool open);

    // Disconnect the camera with the given IP address or not.
    CameraStatus Disconnect();
    CameraStatus Disconnect(const std::string& cameraIp);

    Gocator_Data RetriveData();

    // Shutdown the camera and release resources
    void Shutdown();

private:
    Gocator_Handle gocator;    // Handle for the specific camera
    Gocator_Data data;

    std::vector<CameraInfo> ConvertToCameraInfoList(const Gocator_List& sdkCameraList);

    void RemoveInvalidPoints();
};

#endif // LINE_SCANNER_INTERFACE_H
