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

static void MPI_Ibcast_prolog(void* buffer  MAYBE_UNUSED,
                              int count  MAYBE_UNUSED,
                              MPI_Datatype datatype MAYBE_UNUSED,
                              int root MAYBE_UNUSED,
                              MPI_Comm comm MAYBE_UNUSED,
                              MPI_Request* r MAYBE_UNUSED) {

}

static int MPI_Ibcast_core(void* buffer,
			   int count,
			   MPI_Datatype datatype,
			   int root,
			   MPI_Comm comm,
			   MPI_Request* r) {
  return libMPI_Ibcast(buffer, count, datatype, root, comm, r);
}

static void MPI_Ibcast_epilog(void* buffer  MAYBE_UNUSED,
                              int count  MAYBE_UNUSED,
                              MPI_Datatype datatype MAYBE_UNUSED,
                              int root MAYBE_UNUSED,
                              MPI_Comm comm MAYBE_UNUSED,
                              MPI_Request* r MAYBE_UNUSED) {

}

int MPI_Ibcast(void* buffer,
	       int count,
	       MPI_Datatype datatype,
	       int root,
	       MPI_Comm comm,
	       MPI_Request* r) {
  FUNCTION_ENTRY;
  MPI_Ibcast_prolog(buffer, count, datatype, root, comm, r);
  int ret = MPI_Ibcast_core(buffer, count, datatype, root, comm, r);
  MPI_Ibcast_epilog(buffer, count, datatype, root, comm, r);
  FUNCTION_EXIT;
  return ret;
}

void mpif_ibcast_(void* buf,
		  int* count,
		  MPI_Fint* d,
                  int* root,
		  MPI_Fint* c,
		  MPI_Fint* r,
		  int* error) {
  FUNCTION_ENTRY_("mpi_ibcast_");
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Request c_req = MPI_Request_f2c(*r);

  MPI_Ibcast_prolog(buf, *count, c_type, *root, c_comm, r);
  *error = MPI_Ibcast_core(buf, *count, c_type, *root, c_comm, &c_req);
  *r = MPI_Request_c2f(c_req);
  MPI_Ibcast_epilog(buf, *count, c_type, *root, c_comm, r);
  FUNCTION_EXIT_("mpi_ibcast_");
}
#endif
