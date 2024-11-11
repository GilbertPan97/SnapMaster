# Set the C++ standard
set(CMAKE_CXX_STANDARD 11)

# Find OpenCV package
# find_package(OpenCV REQUIRED PATHS ${THIRD_PARTY}/opencv-4.5.4 NO_DEFAULT_PATH)
find_package(OpenCV REQUIRED)

if(OpenCV_FOUND)
    message(STATUS "OpenCV version: ${OpenCV_VERSION}")
else()
    message(FATAL_ERROR "OpenCV not found. Please install OpenCV.")
endif()

# Define source files and header files
set(SRC_FILES
    KMeansCluster.cpp
    DBSCANCluster.cpp
)

# Specify the library target
add_library(FilterTools ${SRC_FILES})  # Use STATIC for static library

# Include Go_SDK headers and current directory for header files
target_include_directories(FilterTools PUBLIC 
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(FilterTools PUBLIC ${OpenCV_LIBS})

# Add executable for testing and link libraries
if(ON)
    add_executable(TestFilter Samples/TestFilter.cpp)
    target_link_libraries(TestFilter FilterTools)
endif()
