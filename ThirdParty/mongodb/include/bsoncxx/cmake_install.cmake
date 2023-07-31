# Install script for directory: E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/bsoncxx

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
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/bsoncxx/v_noabi" TYPE DIRECTORY FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/src/bsoncxx" FILES_MATCHING REGEX "/[^/]*\\.hpp$")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/bsoncxx/v_noabi/bsoncxx/config" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/config/export.hpp")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/libbsoncxx-3.8.0" TYPE FILE FILES
    "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/libbsoncxx-config.cmake"
    "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/libbsoncxx-config-version.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xdevx" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/Debug/bsoncxx.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/Release/bsoncxx.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/MinSizeRel/bsoncxx.lib")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib" TYPE STATIC_LIBRARY OPTIONAL FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/RelWithDebInfo/bsoncxx.lib")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xruntimex" OR NOT CMAKE_INSTALL_COMPONENT)
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/Debug/bsoncxx.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/Release/bsoncxx.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/MinSizeRel/bsoncxx.dll")
  elseif("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE SHARED_LIBRARY FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/RelWithDebInfo/bsoncxx.dll")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/bsoncxx-3.8.0/bsoncxx_targets.cmake")
    file(DIFFERENT EXPORT_FILE_CHANGED FILES
         "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/bsoncxx-3.8.0/bsoncxx_targets.cmake"
         "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/CMakeFiles/Export/lib/cmake/bsoncxx-3.8.0/bsoncxx_targets.cmake")
    if(EXPORT_FILE_CHANGED)
      file(GLOB OLD_CONFIG_FILES "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/bsoncxx-3.8.0/bsoncxx_targets-*.cmake")
      if(OLD_CONFIG_FILES)
        message(STATUS "Old export file \"$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/cmake/bsoncxx-3.8.0/bsoncxx_targets.cmake\" will be replaced.  Removing files [${OLD_CONFIG_FILES}].")
        file(REMOVE ${OLD_CONFIG_FILES})
      endif()
    endif()
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/bsoncxx-3.8.0" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/CMakeFiles/Export/lib/cmake/bsoncxx-3.8.0/bsoncxx_targets.cmake")
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/bsoncxx-3.8.0" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/CMakeFiles/Export/lib/cmake/bsoncxx-3.8.0/bsoncxx_targets-debug.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/bsoncxx-3.8.0" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/CMakeFiles/Export/lib/cmake/bsoncxx-3.8.0/bsoncxx_targets-minsizerel.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/bsoncxx-3.8.0" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/CMakeFiles/Export/lib/cmake/bsoncxx-3.8.0/bsoncxx_targets-relwithdebinfo.cmake")
  endif()
  if("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/bsoncxx-3.8.0" TYPE FILE FILES "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/CMakeFiles/Export/lib/cmake/bsoncxx-3.8.0/bsoncxx_targets-release.cmake")
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xDevelx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/bsoncxx-3.8.0" TYPE FILE FILES
    "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/bsoncxx-config-version.cmake"
    "E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/bsoncxx-config.cmake"
    )
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/third_party/cmake_install.cmake")
  include("E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/config/cmake_install.cmake")
  include("E:/github-project/ThirdParty/mongo-cxx-driver-r3.8.0/build/src/bsoncxx/test/cmake_install.cmake")

endif()

