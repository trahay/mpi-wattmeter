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

static int MPI_Recv_init_core(void* buffer,
			      int count,
			      MPI_Datatype type,
			      int src,
			      int tag,
			      MPI_Comm comm,
			      MPI_Request* req) {
  return libMPI_Recv_init(buffer, count, type, src, tag, comm, req);
}

static void MPI_Recv_init_epilog(void* buffer MAYBE_UNUSED,
				 int count MAYBE_UNUSED,
				 MPI_Datatype type MAYBE_UNUSED,
				 int src MAYBE_UNUSED,
				 int tag MAYBE_UNUSED,
				 MPI_Comm comm MAYBE_UNUSED,
				 MPI_Fint* req MAYBE_UNUSED) {
}

int MPI_Recv_init(void* buffer,
		  int count,
		  MPI_Datatype type,
		  int src,
		  int tag,
                  MPI_Comm comm,
		  MPI_Request* req) {
  FUNCTION_ENTRY;
  int ret = MPI_Recv_init_core(buffer, count, type, src, tag, comm, req);
  MPI_Recv_init_epilog(buffer, count, type, src, tag, comm, (MPI_Fint*)req);
  FUNCTION_EXIT;
  return ret;
}

void mpif_recv_init_(void* buffer, int* count, MPI_Fint* type, int* src, int* tag,
                     MPI_Fint* comm, MPI_Fint* req, int* error) {
  FUNCTION_ENTRY_("mpi_recv_init_");
  MPI_Datatype c_type = MPI_Type_f2c(*type);
  MPI_Comm c_comm = MPI_Comm_f2c(*comm);
  MPI_Request c_req = MPI_Request_f2c(*req);

  *error = MPI_Recv_init_core(buffer, *count, c_type, *src, *tag, c_comm,
                              &c_req);
  *req = MPI_Request_c2f(c_req);
  MPI_Recv_init_epilog(buffer, *count, c_type, *src, *tag, c_comm, req);
  FUNCTION_EXIT_("mpi_recv_init_");
}
