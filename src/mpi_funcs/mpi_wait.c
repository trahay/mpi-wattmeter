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

static void MPI_Wait_prolog(MPI_Fint* req MAYBE_UNUSED,
                            MPI_Status* s MAYBE_UNUSED) {

}

static int MPI_Wait_core(MPI_Request* req, MPI_Status* s) {
  return libMPI_Wait(req, s);
}


static void MPI_Wait_epilog(MPI_Fint* req MAYBE_UNUSED,
                            MPI_Status* s MAYBE_UNUSED) {
}

int MPI_Wait(MPI_Request* req, MPI_Status* s) {
  FUNCTION_ENTRY;

  MPI_Status ezt_mpi_status;
  if(!s || s == MPI_STATUS_IGNORE) 
    s = &ezt_mpi_status;

  MPI_Wait_prolog((MPI_Fint*)req, s);
  int ret = MPI_Wait_core(req, s);
  MPI_Wait_epilog((MPI_Fint*)req, s);
  FUNCTION_EXIT;
  return ret;
}

void mpif_wait_(MPI_Fint* r, MPI_Fint* s, int* error) {
  FUNCTION_ENTRY_("mpi_wait_");
  MPI_Request c_req = MPI_Request_f2c(*r);
  MPI_Status c_status;
  MPI_Wait_prolog(r, &c_status);
  *error = MPI_Wait_core(&c_req, &c_status);
  MPI_Status_c2f(&c_status, s);
  MPI_Wait_epilog(r, &c_status);
  FUNCTION_EXIT_("mpi_wait_");
}
