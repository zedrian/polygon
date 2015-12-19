find_path(mbedTLS_INCLUDE_DIR
	NAMES mbedtls/ssl.h
	PATHS
		"$ENV{PROGRAMFILES}/mbed TLS"
		"$ENV{PROGRAMW6432}/mbed TLS"
	PATH_SUFFIXES include)

find_library(mbedTLS_LIBRARY
	 NAMES mbedtls
	 PATHS
		"$ENV{PROGRAMFILES}/mbed TLS"
		"$ENV{PROGRAMW6432}/mbed TLS"
	 PATH_SUFFIXES lib)

find_library(mbedCRYPTO_LIBRARY
	 NAMES mbedcrypto
	 PATHS
		"$ENV{PROGRAMFILES}/mbed TLS"
		"$ENV{PROGRAMW6432}/mbed TLS"
	 PATH_SUFFIXES lib)

find_library(mbedX509_LIBRARY
	 NAMES mbedx509
	 PATHS
		"$ENV{PROGRAMFILES}/mbed TLS"
		"$ENV{PROGRAMW6432}/mbed TLS"
	 PATH_SUFFIXES lib)

if(mbedTLS_INCLUDE_DIR AND mbedTLS_LIBRARY)
	set(mbedTLS_FOUND TRUE)
	set(mbedTLS_LIBRARIES ${mbedTLS_LIBRARY} ${mbedX509_LIBRARY} ${mbedCRYPTO_LIBRARY})
	if(WIN32)
		set(mbedTLS_LIBRARIES ${mbedTLS_LIBRARIES} ws2_32)
	endif(WIN32)
endif()

if(mbedTLS_FOUND)
	if(NOT mbedTLS_FIND_QUIETLY)
		message(STATUS "Found mbed TLS: ${mbedTLS_LIBRARIES}")
	endif()
else()
	if(mbedTLS_FIND_REQUIRED)
		message(FATAL_ERROR "mbed TLS was not found")
	endif()
endif()

mark_as_advanced(mbedTLS_INCLUDE_DIR mbedTLS_LIBRARY mbedTLS_LIBRARIES)