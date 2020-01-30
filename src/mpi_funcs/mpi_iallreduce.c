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

static void MPI_Iallreduce_prolog(CONST void* sendbuf  MAYBE_UNUSED,
                                  void* recvbuf  MAYBE_UNUSED,
                                  int count  MAYBE_UNUSED,
                                  MPI_Datatype datatype MAYBE_UNUSED,
                                  MPI_Op op  MAYBE_UNUSED,
                                  MPI_Comm comm MAYBE_UNUSED,
                                  MPI_Request* r MAYBE_UNUSED) {

}

static int MPI_Iallreduce_core(CONST void* sendbuf,
			       void* recvbuf,
			       int count,
                               MPI_Datatype datatype,
			       MPI_Op op,
			       MPI_Comm comm,
                               MPI_Request* r) {
  return libMPI_Iallreduce(sendbuf, recvbuf, count, datatype, op, comm, r);
}

static void MPI_Iallreduce_epilog(CONST void* sendbuf  MAYBE_UNUSED,
                                  void* recvbuf  MAYBE_UNUSED,
                                  int count  MAYBE_UNUSED,
                                  MPI_Datatype datatype MAYBE_UNUSED,
                                  MPI_Op op  MAYBE_UNUSED,
                                  MPI_Comm comm MAYBE_UNUSED,
                                  MPI_Request* r MAYBE_UNUSED) {

}

int MPI_Iallreduce(CONST void* sendbuf,
		   void* recvbuf,
		   int count,
                   MPI_Datatype datatype,
		   MPI_Op op,
		   MPI_Comm comm,
		   MPI_Request* r) {
  FUNCTION_ENTRY;
  MPI_Iallreduce_prolog(sendbuf, recvbuf, count, datatype, op, comm, r);
  int ret = MPI_Iallreduce_core(sendbuf, recvbuf, count, datatype, op, comm, r);
  MPI_Iallreduce_epilog(sendbuf, recvbuf, count, datatype, op, comm, r);
  FUNCTION_EXIT;
  return ret;
}

void mpif_iallreduce_(void* sbuf, void* rbuf, int* count,
                      MPI_Fint* d, MPI_Fint* op,
                      MPI_Fint* c, MPI_Fint* r, int* error) {
  FUNCTION_ENTRY_("mpi_iallreduce_");
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Op c_op = MPI_Op_f2c(*op);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Request c_req = MPI_Request_f2c(*r);

  MPI_Iallreduce_prolog(sbuf, rbuf, *count, c_type, c_op, c_comm, r);
  *error = MPI_Iallreduce_core(sbuf, rbuf, *count, c_type, c_op, c_comm, &c_req);
  *r = MPI_Request_c2f(c_req);
  MPI_Iallreduce_epilog(sbuf, rbuf, *count, c_type, c_op, c_comm, r);
  FUNCTION_EXIT_("mpi_iallreduce_");
}

#endif
