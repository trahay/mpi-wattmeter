/* -*- c-file-style: "GNU" -*- */
/*
 * Copyright © CNRS, INRIA, Université Bordeaux 1, Telecom SudParis
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

#ifdef USE_MPI3

static void MPI_Ibarrier_prolog(MPI_Comm comm MAYBE_UNUSED,
				MPI_Fint* r MAYBE_UNUSED) {
}

static int MPI_Ibarrier_core(MPI_Comm c, MPI_Request* r) {
  return libMPI_Ibarrier(c, r);
}

static void MPI_Ibarrier_epilog(MPI_Comm c MAYBE_UNUSED,
				MPI_Fint* r MAYBE_UNUSED) {

}

int MPI_Ibarrier(MPI_Comm c,
		 MPI_Request* req) {
  FUNCTION_ENTRY;
  MPI_Ibarrier_prolog(c, (MPI_Fint*)req);
  int ret = MPI_Ibarrier_core(c, req);
  MPI_Ibarrier_epilog(c, (MPI_Fint*)req);
  FUNCTION_EXIT;
  return ret;
}

void mpif_ibarrier_(MPI_Fint* c,
		    MPI_Fint* r,
		    int* error) {
  FUNCTION_ENTRY_("mpi_ibarrier_");
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Request c_req = MPI_Request_f2c(*r);

  MPI_Ibarrier_prolog(c_comm, r);
  *error = MPI_Ibarrier_core(c_comm, &c_req);
  *r = MPI_Request_c2f(c_req);
  MPI_Ibarrier_epilog(c_comm, r);
  FUNCTION_EXIT_("mpi_ibarrier_");
}

#endif
