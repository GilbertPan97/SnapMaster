set(CMAKE_C_STANDARD 99)
cmake_policy(SET CMP0111 NEW)           # Set the policy for imported targets

# Find the Go_SDK package (assumed to be defined in the top-level CMake)
# find_package(Go_SDK REQUIRED PATHS ${THIRD_PARTY}/Go_SDK NO_DEFAULT_PATH)
find_package(Go_SDK REQUIRED)

# Define source files and header files
set(SRC_FILES
    Initialize.c
    Discover.c                      # C source file
    Connect.c
    Open.c
    ReceiveProfile.c
    ../CLogger/clog.c
    ../CLogger/logger.c
)

add_library(Go_Driver ${SRC_FILES})

# # CLogger shoule be build before Go_Driver
# add_dependencies(Go_Driver clogger)

message(STATUS "Go_SDK include directory is: ${Go_SDK_INCLUDE_DIRS}")
target_include_directories(Go_Driver PUBLIC
    ${Go_SDK_INCLUDE_DIRS} 
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../CLogger
)

# Create imported library for Go_SDK import libraries
target_link_libraries(Go_Driver PUBLIC ${Go_SDK_IMPORT_LIBS})

# Copy dynamic libraries to the output directory of the library
foreach(DLL ${Go_SDK_DYNAMIC_LIBS})
    get_filename_component(DLL_NAME ${DLL} NAME)      # Get the name of the DLL
    add_custom_command(TARGET Go_Driver POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${DLL}"
            "$<TARGET_FILE_DIR:Go_Driver>/${DLL_NAME}"
        COMMENT "Copying ${DLL_NAME} to the output directory"
    )
endforeach()

if(ON)
    add_executable(GoDriver_test test.c)
    target_link_libraries(GoDriver_test PRIVATE Go_Driver)
    target_include_directories(GoDriver_test PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()