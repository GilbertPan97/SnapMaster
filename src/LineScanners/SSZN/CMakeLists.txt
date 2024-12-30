set(TARGET_NAME Sszn_Driver)

find_package(SR7Link REQUIRED)

set(SRC_FILES CallOneTimes.cpp)

set(HEADER_FILES CallOneTimes.h)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(${TARGET_NAME} ${SRC_FILES})

target_link_libraries(${TARGET_NAME} PUBLIC ${SR7Link_LIBRARIES})

target_include_directories(${TARGET_NAME} PUBLIC
                        ${SR7Link_INCLUDE_DIRS})

if(OFF)
    add_executable(Sszn_test test.cpp)
    target_link_libraries(Sszn_test PRIVATE ${TARGET_NAME})
    target_include_directories(Sszn_test PRIVATE 
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()