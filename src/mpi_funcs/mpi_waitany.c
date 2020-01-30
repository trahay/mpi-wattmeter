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

static void MPI_Waitany_prolog(int count MAYBE_UNUSED,
			       void* reqs MAYBE_UNUSED,
                               int* index  MAYBE_UNUSED,
                               MPI_Status* status  MAYBE_UNUSED,
                               size_t size MAYBE_UNUSED) {
}

static int MPI_Waitany_core(int count,
			    MPI_Request* reqs,
			    int* index,
                            MPI_Status* status) {
  return libMPI_Waitany(count, reqs, index, status);
}


static void MPI_Waitany_epilog(int count MAYBE_UNUSED,
			       MPI_Request* reqs MAYBE_UNUSED,
                               int* index  MAYBE_UNUSED,
                               MPI_Status* status MAYBE_UNUSED,
                               size_t size MAYBE_UNUSED) {
}

int MPI_Waitany(int count,
		MPI_Request* reqs,
		int* index,
		MPI_Status* status) {
  FUNCTION_ENTRY;
  MPI_Status ezt_mpi_status[count];
  if(status == MPI_STATUSES_IGNORE)
    status = ezt_mpi_status;

  MPI_Waitany_prolog(count, reqs, index, status, sizeof(MPI_Request));
  int ret = MPI_Waitany_core(count, reqs, index, status);
  MPI_Waitany_epilog(count, reqs, index, status, sizeof(MPI_Request));
  FUNCTION_EXIT;
  return ret;
}

void mpif_waitany_(int* c,
		   MPI_Fint* r,
		   MPI_Status* s,
		   int* index,
		   int* error) {
  FUNCTION_ENTRY_("mpi_waitany_");
  int i;
  MPI_Waitany_prolog(*c, r, index, s, sizeof(MPI_Fint));

  ALLOCATE_ITEMS(MPI_Request, *c, c_req, p_req);

  for (i = 0; i < *c; i++)
    p_req[i] = MPI_Request_f2c(r[i]);
  *error = MPI_Waitany_core(*c, p_req, index, s);
  for (i = 0; i < *c; i++)
    r[i] = MPI_Request_c2f(p_req[i]);

  MPI_Waitany_epilog(*c, r, index, s, sizeof(MPI_Fint));
  FREE_ITEMS(*c, p_req);
  FUNCTION_EXIT_("mpi_waitany_");
}
