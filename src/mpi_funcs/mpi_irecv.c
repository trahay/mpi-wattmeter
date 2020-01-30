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

static void MPI_Irecv_prolog(void* buf MAYBE_UNUSED,
			     int count MAYBE_UNUSED,
                             MPI_Datatype datatype MAYBE_UNUSED,
			     int src MAYBE_UNUSED,
			     int tag MAYBE_UNUSED,
                             MPI_Comm comm MAYBE_UNUSED,
			     MPI_Fint* req MAYBE_UNUSED) {
}

static int MPI_Irecv_core(void* buf,
			  int count,
			  MPI_Datatype datatype,
			  int src,
                          int tag,
			  MPI_Comm comm,
			  MPI_Request* req) {
  return libMPI_Irecv(buf, count, datatype, src, tag, comm, req);
}


static void MPI_Irecv_epilog(void* buf MAYBE_UNUSED,
			     int count MAYBE_UNUSED,
                             MPI_Datatype datatype MAYBE_UNUSED,
			     int src MAYBE_UNUSED,
			     int tag MAYBE_UNUSED,
                             MPI_Comm comm MAYBE_UNUSED,
			     MPI_Fint* req MAYBE_UNUSED) {
}

int MPI_Irecv(void* buf,
	      int count,
	      MPI_Datatype datatype,
	      int src,
	      int tag,
              MPI_Comm comm,
	      MPI_Request* req) {
  FUNCTION_ENTRY;
  MPI_Irecv_prolog(buf, count, datatype, src, tag, comm, (MPI_Fint*)req);
  int ret = MPI_Irecv_core(buf, count, datatype, src, tag, comm, req);
  MPI_Irecv_epilog(buf, count, datatype, src, tag, comm, (MPI_Fint*)req);
  FUNCTION_EXIT;
  return ret;
}

void mpif_irecv_(void* buf,
		 int* count,
		 MPI_Fint* d,
		 int* src,
		 int* tag,
                 MPI_Fint* c,
		 MPI_Fint* r,
		 int* error) {

  FUNCTION_ENTRY_("mpi_irecv_");
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Request c_req = MPI_Request_f2c(*r);

  MPI_Irecv_prolog(buf, *count, c_type, *src, *tag, c_comm, r);
  *error = MPI_Irecv_core(buf, *count, c_type, *src, *tag, c_comm, &c_req);
  *r = MPI_Request_c2f(c_req);
  MPI_Irecv_epilog(buf, *count, c_type, *src, *tag, c_comm, r);
  FUNCTION_EXIT_("mpi_irecv_");
}
