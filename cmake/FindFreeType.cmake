find_path(
        FreeType_INCLUDE_DIR_ft2build
            ft2build.h
        HINTS
            ENV FreeType_DIR
        PATHS
            "$ENV{PROGRAMFILES}/freetype"
		    "$ENV{PROGRAMW6432}/freetype"
            /usr/X11R6
            /usr/local/X11R6
            /usr/local/X11
            /usr/freeware
            ENV GTKMM_BASEPATH
            [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
            [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
        PATH_SUFFIXES
            include/freetype2
            include
            freetype2)

find_path(
        FreeType_INCLUDE_DIR_freetype2
        NAMES
            freetype/config/ftheader.h
            config/ftheader.h
        HINTS
            ENV FreeType_DIR
        PATHS
            "$ENV{PROGRAMFILES}/freetype"
            "$ENV{PROGRAMW6432}/freetype"
            /usr/X11R6
            /usr/local/X11R6
            /usr/local/X11
            /usr/freeware
            ENV GTKMM_BASEPATH
            [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
            [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
        PATH_SUFFIXES
            include/freetype2
            include
            freetype2
)

find_library(FreeType_LIBRARY
        NAMES
            freetype
            libfreetype
            freetype219
        HINTS
            ENV FreeType_DIR
        PATHS
            "$ENV{PROGRAMFILES}/freetype"
            "$ENV{PROGRAMW6432}/freetype"
            /usr/X11R6
            /usr/local/X11R6
            /usr/local/X11
            /usr/freeware
            ENV GTKMM_BASEPATH
            [HKEY_CURRENT_USER\\SOFTWARE\\gtkmm\\2.4;Path]
            [HKEY_LOCAL_MACHINE\\SOFTWARE\\gtkmm\\2.4;Path]
        PATH_SUFFIXES
            lib
        )

# set the user variables
if(FreeType_INCLUDE_DIR_ft2build AND FreeType_INCLUDE_DIR_freetype2)
    set(FreeType_INCLUDE_DIRS "${FreeType_INCLUDE_DIR_ft2build};${FreeType_INCLUDE_DIR_freetype2}")
    list(REMOVE_DUPLICATES FreeType_INCLUDE_DIRS)
endif()
set(FreeType_LIBRARIES "${FreeType_LIBRARY}")

if(EXISTS "${FreeType_INCLUDE_DIR_freetype2}/freetype/freetype.h")
    set(FreeType_H "${FreeType_INCLUDE_DIR_freetype2}/freetype/freetype.h")
elseif(EXISTS "${FreeType_INCLUDE_DIR_freetype2}/freetype.h")
    set(FreeType_H "${FreeType_INCLUDE_DIR_freetype2}/freetype.h")
endif()

if(FreeType_INCLUDE_DIR_freetype2 AND FreeType_H)
    file(STRINGS "${FreeType_H}" freetype_version_str
            REGEX "^#[\t ]*define[\t ]+FreeType_(MAJOR|MINOR|PATCH)[\t ]+[0-9]+$")

    unset(FreeType_VERSION_STRING)
    foreach(VPART MAJOR MINOR PATCH)
        foreach(VLINE ${freetype_version_str})
            if(VLINE MATCHES "^#[\t ]*define[\t ]+FreeType_${VPART}[\t ]+([0-9]+)$")
                set(FreeType_VERSION_PART "${CMAKE_MATCH_1}")
                if(FreeType_VERSION_STRING)
                    set(FreeType_VERSION_STRING "${FreeType_VERSION_STRING}.${FreeType_VERSION_PART}")
                else()
                    set(FreeType_VERSION_STRING "${FreeType_VERSION_PART}")
                endif()
                unset(FreeType_VERSION_PART)
            endif()
        endforeach()
    endforeach()
endif()


# handle the QUIETLY and REQUIRED arguments and set FreeType_FOUND to TRUE if
# all listed variables are TRUE
include(${CMAKE_CURRENT_LIST_DIR}/FindPackageHandleStandardArgs.cmake)

find_package_handle_standard_args(
        Freetype
        REQUIRED_VARS
            FreeType_LIBRARY
            FreeType_INCLUDE_DIRS
        VERSION_VAR
            FreeType_VERSION_STRING
)

mark_as_advanced(
        FreeType_LIBRARY
        FreeType_INCLUDE_DIR_freetype2
        FreeType_INCLUDE_DIR_ft2build
)