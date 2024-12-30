# Set the C standard (if needed)
set(CMAKE_CXX_STANDARD 11)

add_subdirectory(CLogger)
add_subdirectory(LMI)
add_subdirectory(SSZN)

# Define source files and header files
set(SRC_FILES
    LineScannerInterface.cpp
)

# Specify the library target
add_library(LineScannerInterface ${SRC_FILES})  # Use STATIC for static library

# Include Go_SDK headers and current directory for header files
target_include_directories(LineScannerInterface PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Create imported library for Go_SDK import libraries
target_link_libraries(LineScannerInterface Go_Driver clogger)

if(OFF)
    add_executable(lmiTest Samples/TestLineScanner.cpp)
    target_link_libraries(lmiTest LineScannerInterface clogger)
endif()
