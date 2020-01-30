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

static int MPI_Testany_core(int count,
			    MPI_Request* reqs,
			    int* index,
			    int* flag,
                            MPI_Status* status) {
  return libMPI_Testany(count, reqs, index, flag, status);
}

static void MPI_Testany_epilog(int count  MAYBE_UNUSED,
			       MPI_Request* reqs MAYBE_UNUSED,
			       int* index MAYBE_UNUSED,
			       int* flag MAYBE_UNUSED,
			       MPI_Status* status  MAYBE_UNUSED,
                               size_t size MAYBE_UNUSED) {
}

int MPI_Testany(int count,
		MPI_Request* reqs,
		int* index,
		int* flag,
                MPI_Status* status) {
  FUNCTION_ENTRY;

  MPI_Status ezt_mpi_status[count];
  if(status == MPI_STATUSES_IGNORE)
    status = ezt_mpi_status;

  int ret = MPI_Testany_core(count, reqs, index, flag, status);
  MPI_Testany_epilog(count, reqs, index, flag, status, sizeof(MPI_Request));
  FUNCTION_EXIT;
  return ret;
}

void mpif_testany_(int* count, MPI_Fint* r, int* index, int* flag,
                   MPI_Status* s, int* error) {
  FUNCTION_ENTRY_("mpi_testany_");
  int i;
  ALLOCATE_ITEMS(MPI_Request, *count, c_req, p_req);

  for (i = 0; i < *count; i++)
    p_req[i] = MPI_Request_f2c(r[i]);
  *error = MPI_Testany_core(*count, p_req, index, flag, s);
  for (i = 0; i < *count; i++)
    r[i] = MPI_Request_c2f(p_req[i]);

  MPI_Testany_epilog(*count, r, index, flag, s, sizeof(MPI_Fint));
  FREE_ITEMS(*count, p_req);
  FUNCTION_EXIT_("mpi_testany_");
}
