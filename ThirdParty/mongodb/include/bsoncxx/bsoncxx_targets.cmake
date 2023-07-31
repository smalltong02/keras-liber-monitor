# Generated by CMake

if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}" LESS 2.5)
   message(FATAL_ERROR "CMake >= 2.6.0 required")
endif()
cmake_policy(PUSH)
cmake_policy(VERSION 2.6)
#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Protect against multiple inclusion, which would fail when already imported targets are added once more.
set(_targetsDefined)
set(_targetsNotDefined)
set(_expectedTargets)
foreach(_expectedTarget mongo::bsoncxx_shared)
  list(APPEND _expectedTargets ${_expectedTarget})
  if(NOT TARGET ${_expectedTarget})
    list(APPEND _targetsNotDefined ${_expectedTarget})
  endif()
  if(TARGET ${_expectedTarget})
    list(APPEND _targetsDefined ${_expectedTarget})
  endif()
endforeach()
if("${_targetsDefined}" STREQUAL "${_expectedTargets}")
  unset(_targetsDefined)
  unset(_targetsNotDefined)
  unset(_expectedTargets)
  set(CMAKE_IMPORT_FILE_VERSION)
  cmake_policy(POP)
  return()
endif()
if(NOT "${_targetsDefined}" STREQUAL "")
  message(FATAL_ERROR "Some (but not all) targets in this export set were already defined.\nTargets Defined: ${_targetsDefined}\nTargets not yet defined: ${_targetsNotDefined}\n")
endif()
unset(_targetsDefined)
unset(_targetsNotDefined)
unset(_expectedTargets)


# Create imported target mongo::bsoncxx_shared
add_library(mongo::bsoncxx_shared SHARED IMPORTED)

set_target_properties(mongo::bsoncxx_shared PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/bsoncxx/..;E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/.."
  INTERFACE_LINK_LIBRARIES "Boost::boost"
)

# Import target "mongo::bsoncxx_shared" for configuration "Debug"
set_property(TARGET mongo::bsoncxx_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(mongo::bsoncxx_shared PROPERTIES
  IMPORTED_IMPLIB_DEBUG "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/Debug/bsoncxx.lib"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_DEBUG "mongo::bson_shared"
  IMPORTED_LOCATION_DEBUG "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/Debug/bsoncxx.dll"
  )

# Import target "mongo::bsoncxx_shared" for configuration "Release"
set_property(TARGET mongo::bsoncxx_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(mongo::bsoncxx_shared PROPERTIES
  IMPORTED_IMPLIB_RELEASE "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/Release/bsoncxx.lib"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELEASE "mongo::bson_shared"
  IMPORTED_LOCATION_RELEASE "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/Release/bsoncxx.dll"
  )

# Import target "mongo::bsoncxx_shared" for configuration "MinSizeRel"
set_property(TARGET mongo::bsoncxx_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS MINSIZEREL)
set_target_properties(mongo::bsoncxx_shared PROPERTIES
  IMPORTED_IMPLIB_MINSIZEREL "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/MinSizeRel/bsoncxx.lib"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_MINSIZEREL "mongo::bson_shared"
  IMPORTED_LOCATION_MINSIZEREL "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/MinSizeRel/bsoncxx.dll"
  )

# Import target "mongo::bsoncxx_shared" for configuration "RelWithDebInfo"
set_property(TARGET mongo::bsoncxx_shared APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(mongo::bsoncxx_shared PROPERTIES
  IMPORTED_IMPLIB_RELWITHDEBINFO "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/RelWithDebInfo/bsoncxx.lib"
  IMPORTED_LINK_DEPENDENT_LIBRARIES_RELWITHDEBINFO "mongo::bson_shared"
  IMPORTED_LOCATION_RELWITHDEBINFO "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/RelWithDebInfo/bsoncxx.dll"
  )

# This file does not depend on other imported targets which have
# been exported from the same project but in a separate export set.

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
cmake_policy(POP)
