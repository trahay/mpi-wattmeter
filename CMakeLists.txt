cmake_minimum_required (VERSION 3.17)

project (MPI_WATTMETER
    LANGUAGES C
)
set (MPII_VERSION_MAJOR 0)
set (MPII_VERSION_MINOR 1)
SET(PROJECT_VERSION "${MPII_VERSION_MAJOR}.${MPII_VERSION_MINOR}")
SET(PROJECT_DESCRIPTION "MPI Wattmeter measures the power consumption of MPI programs.")



INCLUDE (CheckLibraryExists)
include(GNUInstallDirs)

find_package(PkgConfig)



cmake_policy(SET CMP0074 NEW)

SET(PKG_CONFIG_LIBDIR  "\${prefix}/lib"  )
SET(PKG_CONFIG_INCLUDEDIR  "\${prefix}/include"  )
SET(PKG_CONFIG_LIBS  "-L\${libdir}"  )
SET(PKG_CONFIG_CFLAGS  "-I\${includedir}"  )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

enable_language(C)
enable_language(Fortran)
option(ENABLE_NVML "Install the NVML  plugin" ON)
option(ENABLE_PERF_EVENT "Install the perf_event plugin" ON)
option(ENABLE_SYSFS "Install the sysfs plugin" ON)

option(ENABLE_FORTRAN "Enable FORTRAN" ON)

find_package (MPI)
if ( NOT MPI_C_FOUND)
  message(ERROR "MPI not found")
endif ()

if(ENABLE_FORTRAN)
  if ( NOT MPI_Fortran_FOUND)
    message(ERROR " MPI fortran not found")
  endif()
endif()

if(ENABLE_NVML)
  find_package (CUDAToolkit)
  add_definitions("-DHAVE_NVML=1")
endif()

option(ENABLE_DEBUG "Enable Debug" ON)
if(ENABLE_DEBUG)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g")
endif()

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/src/mpii_config.h.in"
  "${PROJECT_BINARY_DIR}/src/mpii_config.h"
  )

# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# the RPATH to be used when installing, but only if it's not a system directory
LIST(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
IF("${isSystemDir}" STREQUAL "-1")
  SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
ENDIF("${isSystemDir}" STREQUAL "-1")


# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/src")
add_subdirectory ("src")

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
install(TARGETS DESTINATION bin)
