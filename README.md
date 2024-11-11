# SnapMaster

SnapMaster is a C++ tool designed for capturing images with line-scan cameras. It provides an interface for efficient image acquisition and processing, making it ideal for applications that require high-speed, continuous imaging, such as quality control, object tracking, and measurement.

## Features

- Real-Time Image Capture: Supports high-speed capture with line-scan cameras.
- Flexible Configuration: Configure camera settings for optimal performance.
- Image Display: View captured images in real-time for immediate feedback.
- Efficient Storage: Save images with minimal latency for further analysis.

## Requirements

- Line-scan camera (compatible models depend on specific camera drivers)
- C++17 or later
- CMake 3.10 or later
- Necessary camera drivers and SDK (based on your camera model)

## Installation

1. Clone the repository:
```
git clone https://github.com/yourusername/SnapMaster.git
```
2. Navigate into the project directory:
```
cd SnapMaster
```
3. Create a build directory and run CMake:
```
mkdir build && cd build cmake ..
```
4. Build the project:
```
cmake --build .
```

## Usage

1. Set up your line-scan camera and ensure drivers are properly installed.
2. Run the executable to start capturing images:
```
./SnapMaster
```
3. Adjust camera settings as needed within the application.

## License

This project is licensed under the Apache License 2.0. See the LICENSE file for details.
