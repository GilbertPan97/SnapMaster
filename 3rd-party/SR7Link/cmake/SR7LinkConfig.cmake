# SR7Link version
set(SR7Link_VERSION_MAJOR 1)
set(SR7Link_VERSION_MINOR 0)

# SR7Link include path
set(SR7Link_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../include"
)

# SR7Link lib path
set(SR7Link_LIBRARY_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../lib"
)

# SR7Link bin path
set(SR7Link_EXECUTABLE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../bin"
)

# SR7Link share file path
set(SR7Link_SHARE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../share"
)

# import SR7Link library
find_library(SR7Link_LIBRARY NAMES SR7Link
    PATHS "${SR7Link_LIBRARY_DIRS}"
    NO_DEFAULT_PATH
)

if (SR7Link_LIBRARY)
    set(SR7Link_LIBRARIES ${SR7Link_LIBRARY})
endif ()

# set SR7Link include dir
set(SR7Link_INCLUDE_DIRS ${SR7Link_INCLUDE_DIRS} CACHE PATH "SR7Link include directories")

# set SR7Link lib dir
set(SR7Link_LIBRARY_DIRS ${SR7Link_LIBRARY_DIRS} CACHE PATH "SR7Link library directories")

# set SR7Link exec dir
set(SR7Link_EXECUTABLE_DIRS ${SR7Link_EXECUTABLE_DIRS} CACHE PATH "SR7Link executable directories")

# set SR7Link share dir
set(SR7Link_SHARE_DIRS ${SR7Link_SHARE_DIRS} CACHE PATH "SR7Link share directories")

# set SR7Link library
set(SR7Link_LIBRARIES ${SR7Link_LIBRARIES} CACHE FILEPATH "SR7Link library")

