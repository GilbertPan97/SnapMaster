#include "CallOneTimes.h"

#include <iostream>
#include <string>


void testMemoryAllocation(CallOneTimes& callOneTimes, int profileWidth) {
    callOneTimes.DataMemoryInit(profileWidth);
    std::cout << "Memory initialized successfully!" << std::endl;
}

void testDataSaving(CallOneTimes& callOneTimes, int camId, const std::string& filePath) {
    // Save height data to file
    if (callOneTimes.saveHeightData(filePath, camId) == 0) {
        std::cout << "Height data saved successfully to: " << filePath << std::endl;
    } else {
        std::cerr << "Failed to save height data!" << std::endl;
    }

    // Save intensity data to file
    if (callOneTimes.saveIntensityData(filePath, camId) == 0) {
        std::cout << "Intensity data saved successfully to: " << filePath << std::endl;
    } else {
        std::cerr << "Failed to save intensity data!" << std::endl;
    }

    // Save encoder data to file
    if (callOneTimes.saveEncoderData(filePath, camId) == 0) {
        std::cout << "Encoder data saved successfully to: " << filePath << std::endl;
    } else {
        std::cerr << "Failed to save encoder data!" << std::endl;
    }
}

void testBatchCallback(CallOneTimes& callOneTimes) {
    // Simulate a simple batch callback
    SR7IF_Data mockData;
    // Fill mock data as needed, here just for demonstration
    callOneTimes.BatchOneTimeCallBack(nullptr, &mockData);
    std::cout << "Batch callback executed!" << std::endl;
}

int main() {
    // Create CallOneTimes instance
    CallOneTimes callOneTimes;

    // Initialize memory for testing with a mock profile width
    int profileWidth = 100; // mock profile width
    testMemoryAllocation(callOneTimes, profileWidth);

    // Set some parameters for batch
    callOneTimes.mBatchPoint = 10;
    callOneTimes.mBatchWidth = profileWidth;
    callOneTimes.mXinterVal = 0.5;
    callOneTimes.mYinterVal = 0.5;

    // Test saving data
    std::string filePath = "test_data.ecd"; // Choose your file name and path
    testDataSaving(callOneTimes, 0, filePath); // Test with camera ID 0

    // Test batch callback
    testBatchCallback(callOneTimes);

    // Cleanup
    callOneTimes.deleteDataMemory();
    std::cout << "Memory cleaned up!" << std::endl;

    return 0;
}
