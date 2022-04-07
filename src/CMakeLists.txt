cmake_minimum_required(VERSION 3.1)


add_executable(mpi-interceptor-bin
  mpi_interceptor.c
)

target_include_directories(mpi-interceptor-bin
  PRIVATE
    ${MPI_C_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(mpi-interceptor SHARED
  mpi.c
  mpi_f.f90
  mpi_fortran.c
)

target_include_directories(mpi-interceptor
  PRIVATE
    ${MPI_C_INCLUDE_PATH}
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(mpi-interceptor
  PUBLIC
  dl
  PRIVATE
    ${MPI_Fortran_LIBRARIES}
    ${MPI_C_LIBRARIES}
)

target_compile_options(mpi-interceptor
  PRIVATE
    -Wall -Wextra -Wpedantic
    -DUSE_MPI3
    ${MPI_COMPILE_FLAGS}
)

set_target_properties(mpi-interceptor
  PROPERTIES LINK_FLAGS
  ${MPI_LINK_FLAGS}
)

#----------------------------------------------------

install(TARGETS mpi-interceptor
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

set_target_properties(mpi-interceptor-bin
        PROPERTIES OUTPUT_NAME mpi_interceptor)

install(TARGETS mpi-interceptor-bin
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
