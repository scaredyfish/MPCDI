cmake_minimum_required (VERSION 2.4)
#
# mpcdi
#

project (mpcdi_cmake) 

file(GLOB IO_HDRS IO/*.h) 
file(GLOB IO_SRCS IO/*.cpp) 
source_group(IO\\Headers FILES ${IO_HDRS})
source_group(IO\\Source FILES ${IO_SRCS}) 

file(GLOB CONTAINER_HDRS Container/*.h) 
file(GLOB CONTAINER_SRCS Container/*.cpp) 
source_group(Container\\Headers FILES ${CONTAINER_HDRS})
source_group(Container\\Source FILES ${CONTAINER_SRCS})

file(GLOB BASE_HDRS Base/*.h) 
file(GLOB BASE_SRCS Base/*.cpp) 
source_group(Base\\Headers FILES ${BASE_HDRS})
source_group(Base\\Source FILES ${BASE_SRCS})

file(GLOB UTILS_HDRS Utils/*.h) 
file(GLOB UTILS_SRCS Utils/*.cpp) 
source_group(Utils\\Headers FILES ${UTILS_HDRS})
source_group(Utils\\Source FILES ${UTILS_SRCS})

file(GLOB CREATORS_HDRS Creators/*.h) 
file(GLOB CREATORS_SRCS Creators/*.cpp)
source_group(Creators\\Headers FILES ${CREATORS_HDRS})
source_group(Creators\\Source FILES ${CREATORS_SRCS})

include_directories(Base)
include_directories(IO)
include_directories(Container)
include_directories(Utils)
include_directories(Creators)

include_directories(../ThirdParty/lpng161)
include_directories(${CMAKE_BINARY_DIR}/ThirdParty/lpng161)

add_library (mpcdi SHARED
                ${IO_HDRS}
                ${IO_SRCS}
                ${CONTAINER_HDRS}
				${CONTAINER_SRCS}
                ${BASE_HDRS}
				${BASE_SRCS}
				${UTILS_HDRS}
				${UTILS_SRCS}
				${CREATORS_HDRS}
				${CREATORS_SRCS}
                )
add_definitions(-DMPCDI_EXPORTS)				

# add minizip which it turns pulls in zlib
include_directories(${ZLIB_INCLUDE_DIR}) 
include_directories(${MINIZIP_INCLUDE_DIR})
IF(USE_INCLUDED_ZLIB)
    include_directories(${CMAKE_BINARY_DIR}/ThirdParty/zlib-1.2.7)
ENDIF()
add_dependencies(mpcdi minizip)
target_link_libraries(mpcdi minizip)

# add TINYXML2 library    
include_directories(${TINYXML2_INCLUDE_DIR})
if(TINYXML2_FOUND)
	target_link_libraries(mpcdi ${TINYXML2_LIBRARIES})
ELSE()
	add_dependencies(mpcdi tinyxml2static)
	target_link_libraries(mpcdi tinyxml2static)
ENDIF()				

#add png library
include_directories(${PNG_PNG_INCLUDE_DIR})
if(PNG_FOUND)
	target_link_libraries(mpcdi ${PNG_LIBRARY})
ELSE()
	add_dependencies(mpcdi png16_static)
	target_link_libraries(mpcdi png16_static)
ENDIF()               