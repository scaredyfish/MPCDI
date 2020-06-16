cmake_minimum_required (VERSION 2.4)
project (ReadWriteExample)

cmake_minimum_required (VERSION 2.4)
project (minizip)

IF(NOT DEFINED ZLIB_SOURCE_DIR)
	MESSAGE(FATAL_ERROR "ZLIB_SOURCE_DIR is not defined")
ENDIF()

IF(NOT IS_DIRECTORY ${ZLIB_SOURCE_DIR})
	MESSAGE(FATAL_ERROR "ZLIB_SOURCE_DIR does not point to a valid directory")
ENDIF()

SET(MINIZIP_INCLUDE_DIR ${ZLIB_SOURCE_DIR}/contrib/minizip PARENT_SCOPE)
SET(MINIZIP_INCLUDE_DIR ${ZLIB_SOURCE_DIR}/contrib/minizip)

file(GLOB MINZIP_SRCS ${MINIZIP_INCLUDE_DIR}/*.c) 
list(REMOVE_ITEM MINZIP_SRCS "${MINIZIP_INCLUDE_DIR}/miniunz.c")
list(REMOVE_ITEM MINZIP_SRCS "${MINIZIP_INCLUDE_DIR}/minizip.c")
IF(NOT WIN32)
	list(REMOVE_ITEM MINZIP_SRCS "${MINIZIP_INCLUDE_DIR}/iowin32.c")
ENDIF()


include_directories(${MINIZIP_INCLUDE_DIR})    
IF (NOT USE_INCLUDED_ZLIB)
	include_directories(${ZLIB_INCLUDE_DIR})
ELSE()
	include_directories(${ZLIB_SOURCE_DIR})
	include_directories(${CMAKE_BINARY_DIR}/ThirdParty/zlib-1.2.7)
ENDIF()

add_library(minizip STATIC ${MINZIP_SRCS})

IF(ZLIB_FOUND)
	target_link_libraries(minizip ${ZLIB_LIBRARY})
ELSE()
	add_dependencies(minizip zlib)
	target_link_libraries(minizip zlib)
ENDIF()	
