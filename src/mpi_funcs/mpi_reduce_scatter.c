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

static void MPI_Reduce_scatter_prolog(CONST void* sendbuf  MAYBE_UNUSED,
                                      void* recvbuf  MAYBE_UNUSED,
                                      CONST int* recvcnts  MAYBE_UNUSED,
                                      MPI_Datatype datatype MAYBE_UNUSED,
                                      MPI_Op op  MAYBE_UNUSED,
                                      MPI_Comm comm MAYBE_UNUSED) {
}

static int MPI_Reduce_scatter_core(CONST void* sendbuf,
				   void* recvbuf,
				   CONST int* recvcnts,
				   MPI_Datatype datatype,
				   MPI_Op op,
				   MPI_Comm comm) {
  return libMPI_Reduce_scatter(sendbuf, recvbuf, recvcnts, datatype, op, comm);
}


static void MPI_Reduce_scatter_epilog(CONST void* sendbuf MAYBE_UNUSED,
                                      void* recvbuf  MAYBE_UNUSED,
                                      CONST int* recvcnts  MAYBE_UNUSED,
                                      MPI_Datatype datatype MAYBE_UNUSED,
                                      MPI_Op op  MAYBE_UNUSED,
                                      MPI_Comm comm MAYBE_UNUSED) {
}

int MPI_Reduce_scatter(CONST void* sendbuf,
		       void* recvbuf,
		       CONST int* recvcnts,
		       MPI_Datatype datatype,
		       MPI_Op op,
		       MPI_Comm comm) {
  FUNCTION_ENTRY;
  MPI_Reduce_scatter_prolog(sendbuf, recvbuf, recvcnts, datatype, op, comm);
  int ret = MPI_Reduce_scatter_core(sendbuf, recvbuf, recvcnts, datatype, op,
                                    comm);
  MPI_Reduce_scatter_epilog(sendbuf, recvbuf, recvcnts, datatype, op, comm);
  FUNCTION_EXIT;
  return ret;
}

void mpif_reduce_scatter_(void* sbuf,
			  void* rbuf,
			  int* rcount,
			  MPI_Fint* d,
			  MPI_Fint* op,
			  MPI_Fint* c,
			  int* error) {
  FUNCTION_ENTRY_("mpi_reduce_scatter_");
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Op c_op = MPI_Op_f2c(*op);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  void* c_sbuf = CHECK_MPI_IN_PLACE(sbuf);
  void* c_rbuf = CHECK_MPI_IN_PLACE(rbuf);

  MPI_Reduce_scatter_prolog(c_sbuf, c_rbuf, rcount, c_type, c_op, c_comm);
  *error = MPI_Reduce_scatter_core(c_sbuf, c_rbuf, rcount, c_type, c_op, c_comm);
  MPI_Reduce_scatter_epilog(c_sbuf, c_rbuf, rcount, c_type, c_op, c_comm);
  FUNCTION_EXIT_("mpi_reduce_scatter_");
}
