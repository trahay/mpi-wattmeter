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

/* todo: implement this function ! */

static void MPI_Waitsome_prolog(int incount MAYBE_UNUSED,
				void* reqs MAYBE_UNUSED,
				int* outcount MAYBE_UNUSED,
				int* array_of_indices MAYBE_UNUSED,
				MPI_Status* array_of_statuses MAYBE_UNUSED,
				size_t size MAYBE_UNUSED) {
}

static int MPI_Waitsome_core(int incount,
			     MPI_Request* reqs,
			     int* outcount,
                             int* array_of_indices,
                             MPI_Status* array_of_statuses) {
  return libMPI_Waitsome(incount, reqs, outcount, array_of_indices,
                         array_of_statuses);
}


static void MPI_Waitsome_epilog(int incount MAYBE_UNUSED,
				MPI_Request* reqs MAYBE_UNUSED,
				int* outcount  MAYBE_UNUSED,
				int* array_of_indices  MAYBE_UNUSED,
				MPI_Status* array_of_statuses MAYBE_UNUSED,
				size_t size MAYBE_UNUSED) {
}

int MPI_Waitsome(int incount, MPI_Request* reqs, int* outcount,
                 int* array_of_indices, MPI_Status* array_of_statuses) {
  FUNCTION_ENTRY;
  MPI_Status ezt_mpi_status[incount];
  if(array_of_statuses == MPI_STATUSES_IGNORE)
    array_of_statuses = ezt_mpi_status;

  MPI_Waitsome_prolog(incount, reqs, outcount, array_of_indices,
                      array_of_statuses, sizeof(MPI_Request));
  int ret = MPI_Waitsome_core(incount, reqs, outcount, array_of_indices,
                              array_of_statuses);
  MPI_Waitsome_epilog(incount, reqs, outcount, array_of_indices,
                      array_of_statuses, sizeof(MPI_Request));
  FUNCTION_EXIT;
  return ret;
}

void mpif_waitsome_(int* ic, MPI_Fint* r, int* oc, int* indexes, MPI_Status* s,
                    int* error) {
  FUNCTION_ENTRY_("mpi_waitsome_");
  int i;
  MPI_Waitsome_prolog(*ic, r, oc, indexes, s, sizeof(MPI_Fint));

  ALLOCATE_ITEMS(MPI_Request, *ic, c_req, p_req);

  for (i = 0; i < *ic; i++)
    p_req[i] = MPI_Request_f2c(r[i]);

  *error = MPI_Waitsome_core(*ic, p_req, oc, indexes, s);

  for (i = 0; i < *ic; i++)
    r[i] = MPI_Request_c2f(p_req[i]);

  MPI_Waitsome_epilog(*ic, r, oc, indexes, s, sizeof(MPI_Fint));
  FREE_ITEMS(*ic, p_req);
  FUNCTION_EXIT_("mpi_waitsome_");
}
