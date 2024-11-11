# Specify the minimum CMake version
cmake_minimum_required(VERSION 3.5)

# Find the required Qt5 packages
find_package(Qt5 COMPONENTS Widgets PrintSupport REQUIRED)

# Create a static library for QCustomPlot
add_library(QCustomPlot STATIC
    QCustomPlot.cpp
    QCustomPlot.h
)

# Set the include directories for the QCustomPlot library
target_include_directories(QCustomPlot PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Link the Qt5 Widgets library to QCustomPlot
target_link_libraries(QCustomPlot Qt5::Widgets Qt5::PrintSupport)

# Configure output directories
option(QCUSTOMPLOT_OUTPUT_DIR "Output directory for QCustomPlot compiled binaries" "${CMAKE_SOURCE_DIR}/output")

# If no custom output directory is provided, use the default
if(QCUSTOMPLOT_OUTPUT_DIR STREQUAL "OFF")
    set(QCUSTOMPLOT_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/output")
endif()

# Print the output directory message
message(STATUS "QCustomPlot will be built to: ${QCUSTOMPLOT_OUTPUT_DIR}")

# Set the output directories for the compiled binaries
set_target_properties(QCustomPlot PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${QCUSTOMPLOT_OUTPUT_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${QCUSTOMPLOT_OUTPUT_DIR}/lib
    RUNTIME_OUTPUT_DIRECTORY ${QCUSTOMPLOT_OUTPUT_DIR}/bin
    AUTOMOC ON          # Enable automatic MOC (Meta-Object Compiler)
    AUTORCC ON          # Enable automatic RCC (Resource Compiler)
    AUTOUIC ON
)

# Add examples directory
add_subdirectory(examples)
