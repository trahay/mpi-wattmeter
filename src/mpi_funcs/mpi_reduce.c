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

static void MPI_Reduce_prolog(CONST void* sendbuf  MAYBE_UNUSED,
                              void* recvbuf  MAYBE_UNUSED,
                              int count  MAYBE_UNUSED,
                              MPI_Datatype datatype MAYBE_UNUSED,
                              MPI_Op op  MAYBE_UNUSED,
                              int root MAYBE_UNUSED,
                              MPI_Comm comm MAYBE_UNUSED) {
}

static int MPI_Reduce_core(CONST void* sendbuf,
			   void* recvbuf,
			   int count,
			   MPI_Datatype datatype,
			   MPI_Op op,
			   int root,
                           MPI_Comm comm) {
  return libMPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
}


static void MPI_Reduce_epilog(CONST void* sendbuf  MAYBE_UNUSED,
                              void* recvbuf  MAYBE_UNUSED,
                              int count  MAYBE_UNUSED,
                              MPI_Datatype datatype MAYBE_UNUSED,
                              MPI_Op op  MAYBE_UNUSED,
                              int root MAYBE_UNUSED,
                              MPI_Comm comm MAYBE_UNUSED) {

}

int MPI_Reduce(CONST void* sendbuf,
	       void* recvbuf,
	       int count,
	       MPI_Datatype datatype,
	       MPI_Op op,
	       int root,
	       MPI_Comm comm) {
  FUNCTION_ENTRY;
  MPI_Reduce_prolog(sendbuf, recvbuf, count, datatype, op, root, comm);
  int ret = MPI_Reduce_core(sendbuf, recvbuf, count, datatype, op, root, comm);
  MPI_Reduce_epilog(sendbuf, recvbuf, count, datatype, op, root, comm);
  FUNCTION_EXIT;
  return ret;
}

void mpif_reduce_(void* sbuf,
		  void* rbuf,
		  int* count,
		  MPI_Fint* d,
		  MPI_Fint* op,
		  int* root,
		  MPI_Fint* c,
		  int* error) {
  FUNCTION_ENTRY_("mpi_reduce_");
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Op c_op = MPI_Op_f2c(*op);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  void* c_sbuf = CHECK_MPI_IN_PLACE(sbuf);
  void* c_rbuf = CHECK_MPI_IN_PLACE(rbuf);

  MPI_Reduce_prolog(c_sbuf, c_rbuf, *count, c_type, c_op, *root, c_comm);
  *error = MPI_Reduce_core(c_sbuf, c_rbuf, *count, c_type, c_op, *root, c_comm);
  MPI_Reduce_epilog(c_sbuf, c_rbuf, *count, c_type, c_op, *root, c_comm);
  FUNCTION_EXIT_("mpi_reduce_");
}
