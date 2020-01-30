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

static void MPI_Waitall_prolog(int count MAYBE_UNUSED,
			       void* req MAYBE_UNUSED,
                               MPI_Status* s  MAYBE_UNUSED,
                               size_t size MAYBE_UNUSED) {

}

static int MPI_Waitall_core(int count,
			    MPI_Request* req,
			    MPI_Status* s) {
  return libMPI_Waitall(count, req, s);
}


static void MPI_Waitall_epilog(int count MAYBE_UNUSED,
			       MPI_Request* req MAYBE_UNUSED,
                               MPI_Status* s  MAYBE_UNUSED,
                               size_t size MAYBE_UNUSED) {
}

int MPI_Waitall(int count, MPI_Request* req, MPI_Status* s) {
  FUNCTION_ENTRY;

  MPI_Status ezt_mpi_status[count];
  if(s == MPI_STATUSES_IGNORE)
    s = ezt_mpi_status;
  
  MPI_Waitall_prolog(count, req, s, sizeof(MPI_Request));
  int ret = MPI_Waitall_core(count, req, s);

  MPI_Waitall_epilog(count, req, s, sizeof(MPI_Request));
  FUNCTION_EXIT;
  return ret;
}

void mpif_waitall_(int* c, MPI_Fint* r, MPI_Status* s, int* error) {
  FUNCTION_ENTRY_("mpi_waitall_");
  int i;
  MPI_Waitall_prolog(*c, r, s, sizeof(MPI_Fint));

  /* allocate a MPI_Request array and convert all the fortran requests
   * into C requests
   */
  ALLOCATE_ITEMS(MPI_Request, *c, c_req, p_req);
  for (i = 0; i < *c; i++)
    p_req[i] = MPI_Request_f2c(r[i]);

  /* call the C version of MPI_Wait */
  *error = MPI_Waitall_core(*c, p_req, s);

  /* Since the requests may have been modified by MPI_Waitall,
   * we need to convert them back to Fortran
   */
  for (i = 0; i < *c; i++)
    r[i] = MPI_Request_c2f(p_req[i]);

  MPI_Waitall_epilog(*c, r, s, sizeof(MPI_Fint));

  FREE_ITEMS(*c, p_req);
  FUNCTION_EXIT_("mpi_waitall_");
}
