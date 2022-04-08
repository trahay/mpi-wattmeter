cmake_minimum_required(VERSION 3.1)

# Compile this module with mpicc/mpif90
set(CMAKE_C_COMPILER ${MPI_C_COMPILER})
set(CMAKE_Fortran_COMPILER ${MPI_Fortran_COMPILER})

add_executable(mpi-wattmeter-bin
  mpi_wattmeter.c
  )

target_include_directories(mpi-wattmeter-bin
  PRIVATE
    ${MPI_C_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
)



set(mpi-wattmeter_SOURCES )
list(APPEND mpi-wattmeter_SOURCES
  mpi.c
  mpi-rapl.c
  )

if(ENABLE_FORTRAN)
  # mpif.h may define a bunch of unused parameter which would generate lots of warnings
  list(APPEND mpi-wattmeter_SOURCES
    mpi_f.f90
    mpi_fortran.c)
  
  set(CMAKE_Fortran_FLAGS "${CMAKE_Fortran_FLAGS} -Wno-unused-parameter")
endif()


if(ENABLE_NVML)
    list(APPEND mpi-wattmeter_SOURCES mpi-nvml.c)  
endif()

add_library(mpi-wattmeter SHARED
  ${mpi-wattmeter_SOURCES}
  )

target_include_directories(mpi-wattmeter
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${NVML_INCLUDE_DIR}
)

target_link_libraries(mpi-wattmeter
  PUBLIC
  dl
  PRIVATE
  ${NVML_LIBRARY}
)

target_compile_options(mpi-wattmeter
  PRIVATE
    -Wall -Wextra -Wpedantic
)

#----------------------------------------------------

install(TARGETS mpi-wattmeter
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set_target_properties(mpi-wattmeter-bin
        PROPERTIES OUTPUT_NAME mpi_wattmeter)

install(TARGETS mpi-wattmeter-bin
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
