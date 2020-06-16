cmake_minimum_required (VERSION 2.4)
project (ExampleCreateProfileTypes)

file(GLOB ECPT_HDRS *.h) 
file(GLOB ECPT_SRCS *.cpp) 

include_directories(../../src)    
include_directories(../../src/Base) 
include_directories(../../src/IO)
include_directories(../../src/Container)
include_directories(../../src/Creators)

    
add_executable(ExampleCreateProfileTypes  ${ECPT_SRCS} ${ECPT_HDRS})
target_link_libraries (ExampleCreateProfileTypes mpcdi)