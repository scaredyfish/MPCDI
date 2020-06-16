cmake_minimum_required (VERSION 2.4)
project (ReadWriteExample)

file(GLOB RWE_HDRS *.h) 
file(GLOB RWE_SRCS *.cpp) 

include_directories(../../src)    
include_directories(../../src/Base) 
include_directories(../../src/IO)
include_directories(../../src/Container)
include_directories(../../src/Creators)
include_directories(../../src/Utils)

add_executable(ReadWriteExample  ${RWE_SRCS} ${RWE_HDRS})
target_link_libraries (ReadWriteExample mpcdi)
