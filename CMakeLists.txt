cmake_minimum_required (VERSION 2.4)
project (mpcdi_cmake)

SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

macro(option _name)
    _option (${_name} ${ARGN})
	MARK_AS_ADVANCED( ${_name} )
endmacro()

#============================================================================
# ThirdParty libraries
#============================================================================
_option(USE_INCLUDED_ZLIB "" on)
set(ZLIB_SOURCE_DIR "" CACHE PATH "Points to the zlib source directory (needed for minizip)") #only set on intialization
MARK_AS_ADVANCED( CLEAR ZLIB_SOURCE_DIR )
if (NOT USE_INCLUDED_ZLIB)
    find_package(zlib REQUIRED)
	set(ZLIB_SOURCE_DIR ${ZLIB_SOURCE_DIR} CACHE PATH "Points to the zlib source directory (needed for minizip)" FORCE )
else()
	set(ZLIB_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/zlib-1.2.7")
	set(ZLIB_SOURCE_DIR ${ZLIB_INCLUDE_DIR} CACHE PATH "Points to the zlib source directory set internally" FORCE )
	set(ZLIB_INCLUDE_DIR ${ZLIB_INCLUDE_DIR};${CMAKE_BINARY_DIR}/ThirdParty/zlib-1.2.7)
IF(NOT WIN32) #will foul find(zlib REQUIRED) called by png cmakelists.txt
	set(ZLIB_LIBRARY  ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/libz.so) 
ELSE()	
	set(ZLIB_LIBRARY  ";") 
ENDIF()	
	MARK_AS_ADVANCED(FORCE ZLIB_SOURCE_DIR )
	add_subdirectory (ThirdParty/zlib-1.2.7 EXCLUDE_FROM_ALL)
	SET_PROPERTY(TARGET zlib PROPERTY FOLDER "ThirdParty")
endif()    
add_subdirectory(ThirdParty/minizip)
SET_PROPERTY(TARGET minizip PROPERTY FOLDER "ThirdParty")
    
_option(USE_INCLUDED_PNG "" on)
if (NOT USE_INCLUDED_PNG)
    find_package(png REQUIRED)
else()
    set(PNG_INCLUDE_DIRS "thirdparty/")
    set(PNG_DEFINITIONS -DPNG_STATIC) 
    set(PNG_LIBRARIES "thirdparty/")
	add_subdirectory (ThirdParty/lpng161 EXCLUDE_FROM_ALL)
	SET_PROPERTY(TARGET png16_static PROPERTY FOLDER "ThirdParty")
endif()

_option(USE_INCLUDED_TINYXML2 "" on)
if (NOT USE_INCLUDED_TINYXML2)
	SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR} )
    find_package(tinyxml2 REQUIRED)
else()
    set(TINYXML2_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/ThirdParty/tinyxml2")
	add_subdirectory (ThirdParty/tinyxml2 EXCLUDE_FROM_ALL)
	SET_PROPERTY(TARGET tinyxml2static PROPERTY FOLDER "ThirdParty")
endif()

#============================================================================
# MPCDI
#============================================================================
add_subdirectory (src)

#============================================================================
# Examples
#============================================================================
add_subdirectory (Examples/ReadWriteExample)
SET_PROPERTY(TARGET ReadWriteExample PROPERTY FOLDER "Examples")

add_subdirectory (Examples/ExampleCreateProfileTypes)
SET_PROPERTY(TARGET ExampleCreateProfileTypes PROPERTY FOLDER "Examples")
