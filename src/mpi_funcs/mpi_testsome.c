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

static int MPI_Testsome_core(int incount,
			     MPI_Request* reqs,
			     int* outcount,
                             int* indexes,
			     MPI_Status* statuses) {
  return libMPI_Testsome(incount, reqs, outcount, indexes, statuses);
}

static void MPI_Testsome_epilog(int incount  MAYBE_UNUSED,
				MPI_Request* reqs MAYBE_UNUSED,
                                int* outcount MAYBE_UNUSED,
                                int* indexes  MAYBE_UNUSED,
                                MPI_Status* statuses  MAYBE_UNUSED,
                                size_t size MAYBE_UNUSED) {
}

int MPI_Testsome(int incount,
		 MPI_Request* reqs,
		 int* outcount,
		 int* indexes,
                 MPI_Status* statuses) {
  FUNCTION_ENTRY;
  MPI_Status ezt_mpi_status[incount];
  if(statuses == MPI_STATUSES_IGNORE)
    statuses = ezt_mpi_status;

  int res = MPI_Testsome_core(incount, reqs, outcount, indexes, statuses);
  MPI_Testsome_epilog(incount, reqs, outcount, indexes, statuses,
                      sizeof(MPI_Request));

  FUNCTION_EXIT;
  return res;
}

void mpif_testsome_(int* ic,
		    MPI_Fint* r,
		    int* oc,
		    int* indexes,
		    MPI_Status* s,
                    int* error) {
  FUNCTION_ENTRY_("mpi_testsome_");
  int i;
  ALLOCATE_ITEMS(MPI_Request, *ic, c_req, p_req);

  for (i = 0; i < *ic; i++)
    p_req[i] = MPI_Request_f2c(r[i]);

  *error = MPI_Testsome_core(*ic, p_req, oc, indexes, s);

  for (i = 0; i < *ic; i++)
    r[i] = MPI_Request_c2f(p_req[i]);

  MPI_Testsome_epilog(*ic, r, oc, indexes, s, sizeof(MPI_Fint));

  FREE_ITEMS(*ic, p_req);
  FUNCTION_EXIT_("mpi_testsome_");
}
