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

static void MPI_Start_prolog(MPI_Fint* req MAYBE_UNUSED) {

}

static int MPI_Start_core(MPI_Request* req) {
  return libMPI_Start(req);
}

static void MPI_Start_epilog(MPI_Fint* req MAYBE_UNUSED) {

}

int MPI_Start(MPI_Request* req) {
  FUNCTION_ENTRY;

  MPI_Start_prolog((MPI_Fint*)req);
  int ret = MPI_Start_core(req);
  MPI_Start_epilog((MPI_Fint*)req);

  FUNCTION_EXIT;
  return ret;
}

void mpif_start_(MPI_Fint* req,
		 int* error) {
  FUNCTION_ENTRY_("mpi_start_");
  MPI_Request c_req = MPI_Request_f2c(*req);

  MPI_Start_prolog(req);
  *error = MPI_Start_core(&c_req);

  *req = MPI_Request_c2f(c_req);
  FUNCTION_EXIT_("mpi_start_");
}
