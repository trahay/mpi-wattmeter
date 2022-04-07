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


