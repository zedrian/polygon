find_path(SFML_INCLUDE_DIR
        NAMES SFML/Main.hpp
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES include)

find_library(SFML_AUDO_LIBRARY
        NAMES sfml-audio
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(SFML_GRAPHICS_LIBRARY
        NAMES sfml-graphics
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(SFML_MAIN_LIBRARY
        NAMES sfml-main
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(SFML_NETWORK_LIBRARY
        NAMES sfml-network
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(SFML_SYSTEM_LIBRARY
        NAMES sfml-system
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(SFML_WINDOW_LIBRARY
        NAMES sfml-window
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(FLAC_LIBRARY
        NAMES FLAC
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(FREETYPE_LIBRARY
        NAMES freetype
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(JPEG_LIBRARY
        NAMES jpeg
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(OGG_LIBRARY
        NAMES ogg
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(OPENAL32_LIBRARY
        NAMES openal32
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(VORBIS_LIBRARY
        NAMES vorbis
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(VORBISENC_LIBRARY
        NAMES vorbisenc
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)

find_library(VORBISFILE_LIBRARY
        NAMES vorbisfile
        PATHS
        "$ENV{PROGRAMFILES}/SFML"
        "$ENV{PROGRAMW6432}/SFML"
        /usr/local
        PATH_SUFFIXES lib)


if(SFML_INCLUDE_DIR AND SFML_MAIN_LIBRARY)
    set(SFML_FOUND TRUE)
    set(SFML_LIBRARIES
            ${SFML_AUDIO_LIBRARY} ${SFML_GRAPHICS_LIBRARY} ${SFML_MAIN_LIBRARY} ${SFML_NETWORK_LIBRARY} ${SFML_SYSTEM_LIBRARY} ${SFML_WINDOW_LIBRARY})
    if(WIN32)
        set(SFML_LIBRARIES
                ${SFML_LIBRARIES} ${FLAC_LIBRARY} ${FREETYPE_LIBRARY} ${JPEG_LIBRARY} ${OGG_LIBRARY} ${OPENAL32_LIBRARY} ${VORBIS_LIBRARY} ${VORBISENC_LIBRARY} ${VORBISFILE_LIBRARY})
    endif(WIN32)
endif()

if(SFML_FOUND)
    if(NOT SFML_FIND_QUIETLY)
        message(STATUS "Found SFML: ${SFML_LIBRARIES}")
    endif()
else()
    if(SFML_FIND_REQUIRED)
        message(FATAL_ERROR "SFML was not found")
    endif()
endif()

mark_as_advanced(SFML_INCLUDE_DIR SFML_LIBRARIES)