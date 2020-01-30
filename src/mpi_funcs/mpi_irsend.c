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

static void MPI_Irsend_prolog(CONST void* buf  MAYBE_UNUSED,
                              int count MAYBE_UNUSED,
                              MPI_Datatype datatype MAYBE_UNUSED,
                              int dest MAYBE_UNUSED,
                              int tag MAYBE_UNUSED,
                              MPI_Comm comm MAYBE_UNUSED,
                              MPI_Fint* req MAYBE_UNUSED) {
}

static int MPI_Irsend_core(CONST void* buf,
			   int count,
			   MPI_Datatype datatype,
                           int dest,
			   int tag,
			   MPI_Comm comm,
			   MPI_Request* req) {
  return libMPI_Irsend(buf, count, datatype, dest, tag, comm, req);
}


static void MPI_Irsend_epilog(CONST void* buf  MAYBE_UNUSED,
                              int count MAYBE_UNUSED,
                              MPI_Datatype datatype MAYBE_UNUSED,
                              int dest MAYBE_UNUSED,
                              int tag MAYBE_UNUSED,
                              MPI_Comm comm MAYBE_UNUSED,
                              MPI_Fint* req MAYBE_UNUSED) {

}

int MPI_Irsend(CONST void* buf,
	       int count,
	       MPI_Datatype datatype,
	       int dest,
               int tag,
	       MPI_Comm comm,
	       MPI_Request* req) {
  FUNCTION_ENTRY;
  MPI_Irsend_prolog(buf, count, datatype, dest, tag, comm, (MPI_Fint*)req);
  int ret = MPI_Irsend_core(buf, count, datatype, dest, tag, comm, req);
  MPI_Irsend_epilog(buf, count, datatype, dest, tag, comm, (MPI_Fint*)req);
  FUNCTION_EXIT;
  return ret;
}

void mpif_irsend_(void* buf,
		  int* count,
		  MPI_Fint* d,
		  int* dest,
		  int* tag,
                  MPI_Fint* c,
		  MPI_Fint* r,
		  int* error) {
  FUNCTION_ENTRY_("mpi_irsend_");
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Request c_req = MPI_Request_f2c(*r);

  MPI_Irsend_prolog(buf, *count, c_type, *dest, *tag, c_comm, r);
  *error = MPI_Irsend_core(buf, *count, c_type, *dest, *tag, c_comm, &c_req);
  *r = MPI_Request_c2f(c_req);
  MPI_Irsend_epilog(buf, *count, c_type, *dest, *tag, c_comm, r);
  FUNCTION_EXIT_("mpi_irsend_");
}
