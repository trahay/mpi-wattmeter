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

static void MPI_Ireduce_scatter_prolog(CONST void* sendbuf  MAYBE_UNUSED,
                                       void* recvbuf  MAYBE_UNUSED,
                                       CONST int* recvcnts  MAYBE_UNUSED,
                                       MPI_Datatype datatype MAYBE_UNUSED,
                                       MPI_Op op  MAYBE_UNUSED,
                                       MPI_Comm comm MAYBE_UNUSED,
                                       MPI_Request* r MAYBE_UNUSED) {
}

static int MPI_Ireduce_scatter_core(CONST void* sendbuf,
				    void* recvbuf,
				    CONST int* recvcnts,
                                    MPI_Datatype datatype,
				    MPI_Op op,
				    MPI_Comm comm,
                                    MPI_Request* r) {
  return libMPI_Ireduce_scatter(sendbuf, recvbuf, recvcnts, datatype, op, comm, r);
}

static void MPI_Ireduce_scatter_epilog(CONST void* sendbuf  MAYBE_UNUSED,
                                       void* recvbuf  MAYBE_UNUSED,
                                       CONST int* recvcnts  MAYBE_UNUSED,
                                       MPI_Datatype datatype MAYBE_UNUSED,
                                       MPI_Op op  MAYBE_UNUSED,
                                       MPI_Comm comm MAYBE_UNUSED,
                                       MPI_Request* r MAYBE_UNUSED) {

}

int MPI_Ireduce_scatter(CONST void* sendbuf,
			void* recvbuf,
			CONST int* recvcnts,
                        MPI_Datatype datatype,
			MPI_Op op,
			MPI_Comm comm,
			MPI_Request* r) {
  FUNCTION_ENTRY;
  MPI_Ireduce_scatter_prolog(sendbuf, recvbuf, recvcnts, datatype, op, comm, r);
  int ret = MPI_Ireduce_scatter_core(sendbuf, recvbuf, recvcnts, datatype, op, comm, r);
  MPI_Ireduce_scatter_epilog(sendbuf, recvbuf, recvcnts, datatype, op, comm, r);
  FUNCTION_EXIT;
  return ret;
}

void mpif_ireduce_scatter_(void* sbuf,
			   void* rbuf,
			   int* rcount,
                           MPI_Fint* d,
			   MPI_Fint* op,
                           MPI_Fint* c,
			   MPI_Fint* r,
			   int* error) {
  FUNCTION_ENTRY_("mpi_ireduce_scatter_");
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Op c_op = MPI_Op_f2c(*op);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Request c_req = MPI_Request_f2c(*r);

  MPI_Ireduce_scatter_prolog(sbuf, rbuf, rcount, c_type, c_op, c_comm, r);
  *error = MPI_Ireduce_scatter_core(sbuf, rbuf, rcount, c_type, c_op, c_comm, &c_req);
  *r = MPI_Request_c2f(c_req);
  MPI_Ireduce_scatter_epilog(sbuf, rbuf, rcount, c_type, c_op, c_comm, r);
  FUNCTION_EXIT_("mpi_ireduce_scatter_");
}

#endif
