cmake_minimum_required(VERSION 3.5)

# Define the project name
project(SnapMaster)

# Set the C++ standard to C++11
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Debug)

set(CMAKE_DEBUG_POSTFIX "d" CACHE STRING "add a postfix, usually d on windows")
set(CMAKE_RELEASE_POSTFIX "" CACHE STRING "add a postfix, usually empty on windows")
set(CMAKE_RELWITHDEBINFO_POSTFIX "rd" CACHE STRING "add a postfix, usually empty on windows")
set(CMAKE_MINSIZEREL_POSTFIX "s" CACHE STRING "add a postfix, usually empty on windows")

set(THIRD_PARTY "${CMAKE_SOURCE_DIR}/3rd-party")
set(BUILD_EXAMPLES OFF CACHE BOOL "Build example projects")

# Traverse all folders under the THIRD_PARTY path and add them to CMAKE_PREFIX_PATH
list(APPEND CMAKE_PREFIX_PATH "${THIRD_PARTY}/GO_SDK")
list(APPEND CMAKE_PREFIX_PATH "${THIRD_PARTY}/SR7Link")
list(APPEND CMAKE_PREFIX_PATH "${THIRD_PARTY}/opencv-4.5.4")

# Configure output directories
option(OUTPUT_DIR "Output directory for compiled binaries" "${CMAKE_SOURCE_DIR}/output")
option(VERSION_SHORT "Software version" "1.0.0")

# If no custom output directory is provided, use the default
if(OUTPUT_DIR STREQUAL "OFF")
    set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/output")
endif()

# Set the output directories for the compiled binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR}/lib)

# Add the src directory as a subdirectory
add_subdirectory(src/FilterTools)
add_subdirectory(src/LineScanners)
add_subdirectory(src/QCustomPlot)
add_subdirectory(src/QtAdvancedDocking)
add_subdirectory(src/QtPropertyBrowser)

# Find the required Qt5 packages (Widgets)
find_package(Qt5 COMPONENTS Widgets REQUIRED)

# Add resources
set(QRC_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources.qrc")  # Set the full path to the .qrc file
set(SRC_FILES src/main.cpp src/MainWindow.cpp src/ThreadWorker.cpp)

# Create the main application executable
add_executable(${PROJECT_NAME} ${SRC_FILES} ${QRC_FILE})

# Link the Qt5 Widgets library and others to the main application
target_link_libraries(${PROJECT_NAME} PRIVATE 
                        QtAdvancedDocking 
                        QtPropertyBrowser 
                        QCustomPlot
                        LineScannerInterface
                        FilterTools
)

target_link_libraries(${PROJECT_NAME} PUBLIC Qt5::Widgets iphlpapi)

# Set properties for the main application
set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON          # Enable automatic MOC (Meta-Object Compiler)
    AUTORCC ON          # Enable automatic RCC (Resource Compiler)
    AUTOUIC ON
)

# Print the output directory for verification
message(STATUS "Build output directory: ${OUTPUT_DIR}")

if(WIN32)
    # add window platform resource config macro definition
    include(cmake/WinResource.cmake)
    # Set the path to the ICO file for the application icon
    set(ICON_FILE ${CMAKE_CURRENT_SOURCE_DIR}/icons/app-logo.ico)  

    # Create Windows version resource
    create_win32_resource_version(
        TARGET ${PROJECT_NAME}
        FILENAME ${PROJECT_NAME}
        ICON ${ICON_FILE}
        VERSION "1.0.0"           # Application version
        DESCRIPTION "SnapMaster is a line scanner manager"
        COMPANY_NAME "Shanghai Fanuc Robotics Co., Ltd." # Company name
        COPYRIGHT "Copyright (c) 2024, Shanghai Fanuc Robotics Co. All rights reserved."
    )
endif()
