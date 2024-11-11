set(CMAKE_C_STANDARD 99)

# Define source files and header files
set(SRC_FILES
    logger.c
)

add_library(clogger ${SRC_FILES})

target_include_directories(clogger PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)