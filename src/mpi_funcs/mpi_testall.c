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

static int MPI_Testall_core(int count,
			    MPI_Request* reqs,
			    int* flag,
                            MPI_Status* s) {
  return libMPI_Testall(count, reqs, flag, s);
}

static void MPI_Testall_epilog(int count MAYBE_UNUSED,
			       MPI_Request* reqs MAYBE_UNUSED,
			       int* flag MAYBE_UNUSED,
                               MPI_Status* s  MAYBE_UNUSED,
                               size_t size MAYBE_UNUSED) {
}

int MPI_Testall(int count,
		MPI_Request* reqs,
		int* flag,
		MPI_Status* s) {
  FUNCTION_ENTRY;

  MPI_Status ezt_mpi_status[count];
  if(s == MPI_STATUSES_IGNORE)
    s = ezt_mpi_status;

  int ret = MPI_Testall_core(count, reqs, flag, s);
  MPI_Testall_epilog(count, (void*)reqs, flag, s, sizeof(MPI_Request));

  FUNCTION_EXIT;
  return ret;
}

void mpif_testall_(int* count,
		   MPI_Fint* r,
		   int* index,
		   MPI_Status* s,
                   int* error) {
  FUNCTION_ENTRY_("mpi_testall_");
  int i;
  ALLOCATE_ITEMS(MPI_Request, *count, c_req, p_req);

  for (i = 0; i < *count; i++)
    p_req[i] = MPI_Request_f2c(r[i]);
  *error = MPI_Testall_core(*count, p_req, index, s);
  for (i = 0; i < *count; i++)
    r[i] = MPI_Request_c2f(p_req[i]);

  MPI_Testall_epilog(*count, r, index, s, sizeof(MPI_Fint));
  FREE_ITEMS(*count, p_req);
  FUNCTION_EXIT_("mpi_testall_");
}
