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

static void MPI_Bsend_init_prolog(CONST void* buffer MAYBE_UNUSED,
				  int count MAYBE_UNUSED,
                                  MPI_Datatype type MAYBE_UNUSED,
				  int dest MAYBE_UNUSED,
				  int tag MAYBE_UNUSED,
                                  MPI_Comm comm MAYBE_UNUSED,
				  MPI_Request* req MAYBE_UNUSED) {

}

static int MPI_Bsend_init_core(CONST void* buffer,
			       int count,
			       MPI_Datatype type,
                               int dest,
			       int tag,
			       MPI_Comm comm,
                               MPI_Request* req) {
  return libMPI_Bsend_init(buffer, count, type, dest, tag, comm, req);
}

static void MPI_Bsend_init_epilog(CONST void* buffer MAYBE_UNUSED,
				  int count MAYBE_UNUSED,
                                  MPI_Datatype type MAYBE_UNUSED,
				  int dest MAYBE_UNUSED,
				  int tag MAYBE_UNUSED,
                                  MPI_Comm comm MAYBE_UNUSED,
				  MPI_Request* req MAYBE_UNUSED) {

}

int MPI_Bsend_init(CONST void* buffer,
		   int count,
		   MPI_Datatype type,
		   int dest,
                   int tag,
		   MPI_Comm comm,
		   MPI_Request* req) {
  FUNCTION_ENTRY;
  MPI_Bsend_init_prolog(buffer, count, type, dest, tag, comm, (void*)req);
  int ret = MPI_Bsend_init_core(buffer, count, type, dest, tag, comm, req);
  MPI_Bsend_init_epilog(buffer, count, type, dest, tag, comm, (void*)req);
  FUNCTION_EXIT;
  return ret;
}

void mpif_bsend_init_(void* buffer,
		      int* count,
		      MPI_Fint* type,
		      int* dest,
		      int* tag,
                      MPI_Fint* comm,
		      MPI_Fint* req,
		      int* error) {
  FUNCTION_ENTRY_("mpi_bsend_init_");
  MPI_Datatype c_type = MPI_Type_f2c(*type);
  MPI_Comm c_comm = MPI_Comm_f2c(*comm);
  MPI_Request c_req = MPI_Request_f2c(*req);

  MPI_Bsend_init_prolog(buffer, *count, c_type, *dest, *tag, c_comm, (MPI_Request*)req);

  *error = MPI_Bsend_init_core(buffer, *count, c_type, *dest, *tag, c_comm,
                               &c_req);
  *req = MPI_Request_c2f(c_req);
  MPI_Bsend_init_epilog(buffer, *count, c_type, *dest, *tag, c_comm, (MPI_Request*)req);
  FUNCTION_EXIT_("mpi_bsend_init_");
}
