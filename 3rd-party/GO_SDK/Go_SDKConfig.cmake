# Go_SDKConfig.cmake

include(CMakeFindDependencyMacro)

# Use CMAKE_CURRENT_LIST_DIR to get the directory where this .cmake file is located
set(Go_SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}")

# Set the include directories relative to the .cmake file's location
set(Go_SDK_INCLUDE_DIRS 
    "${Go_SDK_ROOT}/Gocator/GoSdk"
    "${Go_SDK_ROOT}/Platform/kApi"
)

# Determine static and dynamic library paths based on the build type
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(Go_SDK_LIB_DIR "${Go_SDK_ROOT}/lib/win64d")   # Debug static lib directory
    set(Go_SDK_DYNAMIC_LIB_DIR "${Go_SDK_ROOT}/bin/win64d")   # Debug dynamic lib directory
else()
    set(Go_SDK_LIB_DIR "${Go_SDK_ROOT}/lib/win64")    # Release static lib directory
    set(Go_SDK_DYNAMIC_LIB_DIR "${Go_SDK_ROOT}/bin/win64")    # Release dynamic lib directory
endif()

# Check if the import library directory exists
if(EXISTS "${Go_SDK_LIB_DIR}")
    set(Go_SDK_IMPORT_LIBS
        "${Go_SDK_LIB_DIR}/kApi.lib"       # kApi import Debug lib
        "${Go_SDK_LIB_DIR}/GoSdk.lib"      # GoSdk import Debug lib
    )
else()
    message(FATAL_ERROR "Static library directory does not exist: ${Go_SDK_LIB_DIR}")
endif()

# Check if the dynamic library directory exists
if(EXISTS "${Go_SDK_DYNAMIC_LIB_DIR}")
    set(Go_SDK_DYNAMIC_LIBS
        "${Go_SDK_DYNAMIC_LIB_DIR}/kApi.dll"       # kApi Debug DLL
        "${Go_SDK_DYNAMIC_LIB_DIR}/GoSdk.dll"      # GoSdk Debug DLL
    )
else()
    message(FATAL_ERROR "Dynamic library directory does not exist: ${Go_SDK_DYNAMIC_LIB_DIR}")
endif()

# Create imported libraries for dynamic libraries
set(Go_SDK_LIBRARIES "")
foreach(DLL ${Go_SDK_DYNAMIC_LIBS})
    get_filename_component(DLL_NAME ${DLL} NAME_WE)      # Get the name of the DLL
    set(NAMESPACE_NAME "Go_SDK::${DLL_NAME}")

    # Check if the DLL exists
    if(EXISTS "${DLL}")
        # Get the directory of the DLL
        get_filename_component(DLL_DIR ${DLL} DIRECTORY)

        # Add the custom library
        add_library(${NAMESPACE_NAME} SHARED IMPORTED)
        set_target_properties(${NAMESPACE_NAME} PROPERTIES
            IMPORTED_LOCATION ${DLL}  # Set the imported location
            IMPORTED_IMPLIB ${Go_SDK_LIB_DIR}/${DLL_NAME}.lib
        )
        
        # Add the imported library to Go_SDK_LIBRARIES
        list(APPEND Go_SDK_LIBRARIES ${NAMESPACE_NAME})
    else()
        message(WARNING "DLL not found: ${DLL}. Skipping import library creation.")
    endif()
endforeach()

# Export the target variables
set(Go_SDK_INCLUDE_DIRS ${Go_SDK_INCLUDE_DIRS} CACHE PATH "Go_SDK include directories")
set(Go_SDK_IMPORT_LIBS ${Go_SDK_IMPORT_LIBS} CACHE PATH "Go_SDK static libraries")
set(Go_SDK_DYNAMIC_LIBS ${Go_SDK_DYNAMIC_LIBS} CACHE PATH "Go_SDK dynamic libraries")

# Install the config file
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Go_SDKConfig.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/Go_SDK)
