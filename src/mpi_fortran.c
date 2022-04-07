/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright (C) CNRS, INRIA, Universite Bordeaux 1, Telecom SudParis
 * See COPYING in top-level directory.
 */

#include "mpii.h"
#include <mpi.h>

/* For MPI_Init and MPI_Init_thread, we *have* to call the Fortran version
 * of the function. Once it is done, we can call the __mpi_init_generic function
 */
void __mpi_init_generic();

void mpif_init_(void* error) {
  libmpi_init_((int*)error);
  __mpi_init_generic();
}

void mpif_init_thread_(int* r, int* p, int* error) {
  libmpi_init_thread_(r, p, error);
  __mpi_init_generic();
}

/* For all the remaining Fortran functions, we can just call the C version */
void mpif_finalize_(int* error) {
  *error = MPI_Finalize();
}
