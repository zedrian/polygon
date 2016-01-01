find_path(FreeType_INCLUDE_DIR
        NAMES freetype/freetype.h
        PATHS
            "$ENV{PROGRAMFILES}/freetype"
            "$ENV{PROGRAMW6432}/freetype"
        PATH_SUFFIXES include/freetype2)

find_library(FreeType_LIBRARY
        NAMES freetype
        PATHS
            "$ENV{PROGRAMFILES}/freetype"
            "$ENV{PROGRAMW6432}/freetype"
        PATH_SUFFIXES lib)

if(FreeType_INCLUDE_DIR AND FreeType_LIBRARY)
    set(FreeType_FOUND TRUE)
endif()

if(FreeType_FOUND)
    if(NOT FreeType_FIND_QUIETLY)
        message(STATUS "Found FreeType: ${FreeType_LIBRARY}")
    endif()
else()
    if(FreeType_FIND_REQUIRED)
        message(FATAL_ERROR "FreeType was not found")
    endif()
endif()

mark_as_advanced(FreeType_INCLUDE_DIR FreeType_LIBRARY)