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

static int MPI_Test_core(MPI_Request* req,
			 int* a,
			 MPI_Status* s) {
  return libMPI_Test(req, a, s);
}

static void MPI_Test_epilog(MPI_Fint* req MAYBE_UNUSED,
			    int* a MAYBE_UNUSED,
                            MPI_Status* s MAYBE_UNUSED) {
}

int MPI_Test(MPI_Request* req,
	     int* a,
	     MPI_Status* s) {
  FUNCTION_ENTRY;

  MPI_Status ezt_mpi_status;
  if(!s || s == MPI_STATUS_IGNORE) 
    s = &ezt_mpi_status;

  int res = MPI_Test_core(req, a, s);
  MPI_Test_epilog((MPI_Fint*)req, a, s);
  FUNCTION_EXIT;
  return res;
}

void mpif_test_(MPI_Fint* r,
		int* f,
		MPI_Fint* s,
		int* error) {
  FUNCTION_ENTRY_("mpi_test_");
  MPI_Request c_req = MPI_Request_f2c(*r);
  MPI_Status c_status;

  *error = MPI_Test_core(&c_req, f, &c_status);
  *r = MPI_Request_c2f(c_req);

  if (*f) {
    MPI_Status_c2f(&c_status, s);
  }

  MPI_Test_epilog(r, f, &c_status);
  FUNCTION_EXIT_("mpi_test_");
}
