# Install script for directory: E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/install")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mongocxx/v_noabi" TYPE DIRECTORY FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/mongocxx" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mongocxx/v_noabi/mongocxx/config" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/config/export.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libmongocxx-3.8.0" TYPE FILE FILES
    "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/libmongocxx-config.cmake"
    "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/libmongocxx-config-version.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/Debug/mongocxx.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/Release/mongocxx.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/MinSizeRel/mongocxx.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/RelWithDebInfo/mongocxx.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xruntimex" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/Debug/mongocxx.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/Release/mongocxx.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/MinSizeRel/mongocxx.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/RelWithDebInfo/mongocxx.dll")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/mongocxx-3.8.0/mongocxx_targets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/mongocxx-3.8.0/mongocxx_targets.cmake"
         "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/CMakeFiles/Export/lib/cmake/mongocxx-3.8.0/mongocxx_targets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/mongocxx-3.8.0/mongocxx_targets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/mongocxx-3.8.0/mongocxx_targets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/mongocxx-3.8.0" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/CMakeFiles/Export/lib/cmake/mongocxx-3.8.0/mongocxx_targets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/mongocxx-3.8.0" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/CMakeFiles/Export/lib/cmake/mongocxx-3.8.0/mongocxx_targets-debug.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/mongocxx-3.8.0" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/CMakeFiles/Export/lib/cmake/mongocxx-3.8.0/mongocxx_targets-minsizerel.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/mongocxx-3.8.0" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/CMakeFiles/Export/lib/cmake/mongocxx-3.8.0/mongocxx_targets-relwithdebinfo.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/mongocxx-3.8.0" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/CMakeFiles/Export/lib/cmake/mongocxx-3.8.0/mongocxx_targets-release.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/mongocxx-3.8.0" TYPE FILE FILES
    "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/mongocxx-config-version.cmake"
    "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/mongocxx-config.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/config/cmake_install.cmake")
  include("E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/mongocxx/test/cmake_install.cmake")

endif()

