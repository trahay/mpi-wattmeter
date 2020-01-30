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

static void MPI_Cancel_prolog(MPI_Fint* req MAYBE_UNUSED) {
}

static int MPI_Cancel_core(MPI_Request* request) {
  return libMPI_Cancel(request);
}

int MPI_Cancel(MPI_Request* req) {
  FUNCTION_ENTRY;
  MPI_Cancel_prolog((MPI_Fint*)req);
  int ret = MPI_Cancel_core(req);
  FUNCTION_EXIT;
  return ret;
}

void mpif_cancel_(MPI_Fint* r, int* error) {
  FUNCTION_ENTRY_("mpi_cancel_");
  MPI_Request c_req = MPI_Request_f2c(*r);
  MPI_Cancel_prolog(r);
  *error = MPI_Cancel_core(&c_req);
  FUNCTION_EXIT_("mpi_cancel_");
}
