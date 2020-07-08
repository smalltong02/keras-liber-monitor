cmake_minimum_required(VERSION 2.6)
project(bison)

set(SOURCES
	bson.c
	encoding.c
	numbers.c
)

set ( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT" )
set ( CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT" )
set ( CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_RELEASE} /MTd" )
set ( CMAKE_C_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_RELEASE} /MTd" )

add_library(bson STATIC ${SOURCES})
