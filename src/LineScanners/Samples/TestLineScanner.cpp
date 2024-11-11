#include "../LineScannerInterface.h"
#include <iostream>
#include <vector>

int main() {
    LineScannerInterface lineScanner;
    std::vector<CameraInfo> cameraList;
    CameraStatus status;

    // Scan for available cameras
    status = lineScanner.Scan(cameraList);
    if (status != CameraStatus::Go_OK) {
        std::cerr << "Failed to scan for cameras." << std::endl;
        lineScanner.Shutdown();
        return EXIT_FAILURE;
    }
    // Print the discovered cameras
    std::cout << "Discovered Cameras:" << std::endl;
    for (const auto& camera : cameraList) {
        std::cout << "Camera ID: " << camera.id << ", IP Address: " << camera.ipAddress << std::endl;
    }

    // Initialize the line scanner (use a valid IP address if needed)
    status = lineScanner.Connect("192.168.1.11");
    if (status != CameraStatus::Go_OK) {
        std::cerr << "Failed to initialize line scanner." << std::endl;
        return EXIT_FAILURE;
    }

    // Start the line scanner
    status = lineScanner.SetStatus(true);
    if (status != CameraStatus::Go_OK) {
        std::cerr << "Failed to start line scanner." << std::endl;
        return EXIT_FAILURE;
    }

    // Stop the line scanner
    lineScanner.GrabOnce();
    
    // Shutdown the line scanner
    lineScanner.Shutdown();

    return EXIT_SUCCESS;
}
