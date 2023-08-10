cmake_minimum_required(VERSION 3.17)

# Compile this module with mpicc/mpif90
set(CMAKE_C_COMPILER ${MPI_C_COMPILER})
set(CMAKE_Fortran_COMPILER ${MPI_Fortran_COMPILER})


# libmpi-wattmeter-core
add_library(mpi-wattmeter-core SHARED mpi-plugin.c)
install(TARGETS mpi-wattmeter-core LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

# mpi-wattmeter-bin
add_executable(mpi-wattmeter-bin  mpi_wattmeter.c )
target_link_libraries(mpi-wattmeter-bin PRIVATE mpi-wattmeter-core)

target_include_directories(mpi-wattmeter-bin
  PRIVATE
    ${MPI_C_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

set_target_properties(mpi-wattmeter-bin
        PROPERTIES OUTPUT_NAME mpi_wattmeter)

install(TARGETS mpi-wattmeter-bin
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# libmpi-wattmeter
set(mpi-wattmeter_SOURCES )
list(APPEND mpi-wattmeter_SOURCES
  mpi.c
  mpi-measurement.c)

if(ENABLE_FORTRAN)
  # mpif.h may define a bunch of unused parameter which would generate lots of warnings
  list(APPEND mpi-wattmeter_SOURCES
    mpi_f.f90
    mpi_fortran.c)
  
  set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wno-unused-parameter")
endif()

add_library(mpi-wattmeter SHARED ${mpi-wattmeter_SOURCES})

target_include_directories(mpi-wattmeter PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(mpi-wattmeter
  PUBLIC
    dl
  PRIVATE
    mpi-wattmeter-core
    $<$<BOOL:${ENABLE_NVML}>:CUDA::nvml>
)
install(TARGETS mpi-wattmeter
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# libmpi-wattmeter-nvml
if(ENABLE_NVML)
  add_library(mpi-wattmeter-nvml SHARED mpi-nvml.c)
  target_link_libraries(mpi-wattmeter-nvml PRIVATE CUDA::nvml mpi-wattmeter-core) 
  install(TARGETS mpi-wattmeter-nvml LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if(ENABLE_PERF_EVENT)
  add_library(mpi-wattmeter-perf-event SHARED mpi-perf_event.c)
  target_link_libraries(mpi-wattmeter-perf-event PRIVATE mpi-wattmeter-core) 
  install(TARGETS mpi-wattmeter-perf-event LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

if(ENABLE_SYSFS)
  add_library(mpi-wattmeter-sysfs SHARED mpi-sysfs.c)
  target_link_libraries(mpi-wattmeter-sysfs PRIVATE mpi-wattmeter-core) 
  install(TARGETS mpi-wattmeter-sysfs LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
endif()

#----------------------------------------------------
