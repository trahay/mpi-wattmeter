cmake_minimum_required(VERSION 3.1)


add_executable(mpi-wattmeter-bin
  mpi_wattmeter.c
  )

target_include_directories(mpi-wattmeter-bin
  PRIVATE
    ${MPI_C_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(mpi-wattmeter SHARED
  mpi.c
  mpi_f.f90
  mpi_fortran.c
  mpi-rapl.c
  )

target_include_directories(mpi-wattmeter
  PRIVATE
    ${MPI_C_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mpi-wattmeter
  PUBLIC
  dl
  PRIVATE
    ${MPI_Fortran_LIBRARIES}
    ${MPI_C_LIBRARIES}
)

target_compile_options(mpi-wattmeter
  PRIVATE
    -Wall -Wextra -Wpedantic
    -DUSE_MPI3
    ${MPI_COMPILE_FLAGS}
)

set_target_properties(mpi-wattmeter
  PROPERTIES LINK_FLAGS
  ${MPI_LINK_FLAGS}
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
