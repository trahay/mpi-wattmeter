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

static int MPI_Iprobe_core(int source MAYBE_UNUSED,
			   int tag MAYBE_UNUSED,
			   MPI_Comm comm MAYBE_UNUSED,
			   int* flag MAYBE_UNUSED,
                           MPI_Status* status) {
  return libMPI_Iprobe(source, tag, comm, flag, status);
}

static void MPI_Iprobe_epilog(int source MAYBE_UNUSED,
                              int tag MAYBE_UNUSED,
                              MPI_Comm comm MAYBE_UNUSED,
			      int* flag MAYBE_UNUSED,
                              MPI_Status* status MAYBE_UNUSED) {
}

int MPI_Iprobe(int source,
	       int tag,
	       MPI_Comm comm,
	       int* flag,
               MPI_Status* status) {
  FUNCTION_ENTRY;
  int ret = MPI_Iprobe_core(source, tag, comm, flag, status);
  MPI_Iprobe_epilog(source, tag, comm, flag, status);

  FUNCTION_EXIT;
  return ret;
}

void mpif_iprobe_(int* source, int* tag, MPI_Fint* comm, int* flag,
                  MPI_Status* status, int* err) {
  FUNCTION_ENTRY_("mpi_iprobe_");
  MPI_Comm c_comm = MPI_Comm_f2c(*comm);
  *err = MPI_Iprobe_core(*source, *tag, c_comm, flag, status);
  MPI_Iprobe_epilog(*source, *tag, c_comm, flag, status);
  FUNCTION_EXIT_("mpi_iprobe_");
}
