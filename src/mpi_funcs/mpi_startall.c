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

static void MPI_Startall_prolog(int count MAYBE_UNUSED,
				void* req MAYBE_UNUSED,
				size_t size MAYBE_UNUSED) {
}

static int MPI_Startall_core(int count,
			     MPI_Request* req) {
  return libMPI_Startall(count, req);
}

int MPI_Startall(int count,
		 MPI_Request* req) {
  FUNCTION_ENTRY;

  MPI_Startall_prolog(count, req, sizeof(MPI_Request));
  int ret = MPI_Startall_core(count, req);

  FUNCTION_EXIT;
  return ret;
}

void mpif_startall_(int* count,
		    MPI_Fint* reqs,
		    int* error) {
  FUNCTION_ENTRY_("mpi_startall_");
  int i;
  ALLOCATE_ITEMS(MPI_Request, *count, c_req, p_req);

  for (i = 0; i < *count; i++)
    p_req[i] = MPI_Request_f2c(reqs[i]);

  MPI_Startall_prolog(*count, reqs, sizeof(MPI_Fint));
  *error = MPI_Startall_core(*count, p_req);

  for (i = 0; i < *count; i++)
    reqs[i] = MPI_Request_c2f(p_req[i]);

  FREE_ITEMS(*count, p_req);
  FUNCTION_EXIT_("mpi_startall_");
}
