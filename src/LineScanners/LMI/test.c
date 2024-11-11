#include "GocatorCommon.h"

#include <GoSdk/GoSdk.h>
#include <stdio.h>

int main() {
    // Create an instance of GoSdk_CameraList
    Gocator_List cList;
    cList.cam_info = NULL;  // Initialize the camera pointer to NULL
    cList.count = 0;       // Initialize the count of discovered cameras

    // Call the DiscoverCameras function to find available cameras
    kStatus status = Gocator_Discover(&cList);
    if (status != kOK) {
        printf("Failed to discover cameras. Status: %d\n", status);
        return EXIT_FAILURE; // Exit if discovery fails
    }

    // Print the discovered camera information
    printf("Discovered Cameras:\n");
    for (size_t i = 0; i < cList.count; i++) {
        printf("Camera ID: %d, IP Address: %s\n", cList.cam_info[i].id, \
            cList.cam_info[i].ipAddress);
    }

    // Initialize the Gocator handle to hold camera connection info
    Gocator_Handle handle = {0};
    Gocator_Data data;              // Structure to hold received profile data

    // Initialize the Gocator system with the IP address of the first discovered camera
    status = Gocator_Initialize(&handle);
    if (status != kOK) {
        printf("Initialization Error: %d\n", status); // Print error if initialization fails
        return -1; // Exit with error code
    }
    status = Gocator_Connect(&handle, cList.cam_info[0].ipAddress);
    status = Gocator_Open(&handle);
    
    // // Receive profile data from the Gocator sensor
    status = Gocator_ReceiveProfileData(&handle, &data);
    if (status == kOK) {
        // If profile data is successfully received, print the number of valid points
        printf("Received %zu valid profile points.\n", data.pointCount);
        // Process the profile data as needed (additional processing code can be added here)
        free(data.profileBuffer); // Free the allocated memory for the profile buffer
    } else {
        // Print error if receiving profile data fails
        printf("Error receiving profile data: %d\n", status);
    }

    status = Gocator_Close(&handle);
    status = Gocator_DisConnect(&handle, cList.cam_info[0].ipAddress);

    // Clean up resources associated with the Gocator handle
    Gocator_Cleanup(&handle);

    // Free the memory allocated for the camera list to prevent memory leaks
    Gocator_FreeList(&cList);

    printf("Press any key to continue...\n"); // Prompt user to press a key before exiting

    getchar(); // Wait for user input

    return EXIT_SUCCESS; // Indicate successful execution
}
