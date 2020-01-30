/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright (C) CNRS, INRIA, Universite Bordeaux 1, Telecom SudParis
 * See COPYING in top-level directory.
 */

#ifndef _REENTRANT
#define _REENTRANT
#endif

#include "mpii.h"

#include <dlfcn.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>

static void MPI_Barrier_prolog(MPI_Comm c MAYBE_UNUSED) {
}

static int MPI_Barrier_core(MPI_Comm c) {
  return libMPI_Barrier(c);
}

static void MPI_Barrier_epilog(MPI_Comm c MAYBE_UNUSED) {
}

int MPI_Barrier(MPI_Comm c) {
  FUNCTION_ENTRY;

  MPI_Barrier_prolog(c);
  int ret = MPI_Barrier_core(c);
  MPI_Barrier_epilog(c);

  FUNCTION_EXIT;
  return ret;
}

void mpif_barrier_(MPI_Fint* c, int* error) {
  FUNCTION_ENTRY_("mpi_barrier_");
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Barrier_prolog(c_comm);
  *error = MPI_Barrier_core(c_comm);
  MPI_Barrier_epilog(c_comm);
  FUNCTION_EXIT_("mpi_barrier_");
}
