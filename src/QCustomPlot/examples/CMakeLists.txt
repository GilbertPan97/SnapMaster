# Set the minimum required CMake version
cmake_minimum_required(VERSION 3.5)

# Check if the BUILD_EXAMPLES variable is defined
if(NOT DEFINED BUILD_EXAMPLES)
    # Set the default value to OFF if not defined
    set(BUILD_EXAMPLES OFF CACHE BOOL "Build example projects")
endif()

# Find the required Qt5 packages
find_package(Qt5 COMPONENTS Widgets PrintSupport REQUIRED)

# Specify the examples directory
set(EXAMPLES_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# Collect all subdirectories in the examples directory
file(GLOB EXAMPLE_DIRS RELATIVE ${EXAMPLES_DIR} ${EXAMPLES_DIR}/*)
message(STATUS "Example directories found: ${EXAMPLE_DIRS}")

# Create a list to hold example names
set(EXAMPLE_NAMES "")

if(BUILD_EXAMPLES)
    foreach(EXAMPLE_DIR ${EXAMPLE_DIRS})
        if(IS_DIRECTORY ${EXAMPLES_DIR}/${EXAMPLE_DIR})  # Check if it's a directory
            message(STATUS "Included QCP example directory: ${EXAMPLE_DIR}")

            # Collect all .cpp files in the example directory
            file(GLOB SOURCES ${EXAMPLES_DIR}/${EXAMPLE_DIR}/*.cpp)

            # Collect all .ui files in the example directory
            file(GLOB UI_FILES ${EXAMPLES_DIR}/${EXAMPLE_DIR}/*.ui)

            # Create an executable for each example
            get_filename_component(EXAMPLE_NAME ${EXAMPLE_DIR} NAME)
            add_executable(${EXAMPLE_NAME} ${SOURCES} ${UI_FILES})

            # Enable automatic MOC, RCC, and UI handling
            set_target_properties(${EXAMPLE_NAME} PROPERTIES
                AUTOMOC ON
                AUTOUIC ON
                AUTORCC ON
            )

            # Link the QCustomPlot library and Qt5 Widgets library
            target_link_libraries(${EXAMPLE_NAME} QCustomPlot Qt5::Widgets Qt5::PrintSupport)

            # Set the include directories for the example
            target_include_directories(${EXAMPLE_NAME} PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/..  # Include QCustomPlot headers
            )

            # Add example name to the list
            list(APPEND EXAMPLE_NAMES ${EXAMPLE_NAME})
        else()
            message(WARNING "Not a directory: ${EXAMPLES_DIR}/${EXAMPLE_DIR}")
        endif()
    endforeach()

    # Print out all included example project names
    message(STATUS "Included example projects: ${EXAMPLE_NAMES}")
else()
    message(STATUS "Example projects will not be built.")
endif()
