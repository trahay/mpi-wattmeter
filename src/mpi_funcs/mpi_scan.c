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

static void MPI_Scan_prolog(CONST void* sendbuf  MAYBE_UNUSED,
                            void* recvbuf  MAYBE_UNUSED,
                            int count  MAYBE_UNUSED,
                            MPI_Datatype datatype MAYBE_UNUSED,
                            MPI_Op op  MAYBE_UNUSED,
                            MPI_Comm comm MAYBE_UNUSED) {

}

static int MPI_Scan_core(CONST void* sendbuf,
			 void* recvbuf,
			 int count,
			 MPI_Datatype datatype,
			 MPI_Op op,
			 MPI_Comm comm) {
  return libMPI_Scan(sendbuf, recvbuf, count, datatype, op, comm);
}


static void MPI_Scan_epilog(CONST void* sendbuf  MAYBE_UNUSED,
                            void* recvbuf  MAYBE_UNUSED,
                            int count  MAYBE_UNUSED,
                            MPI_Datatype datatype MAYBE_UNUSED,
                            MPI_Op op  MAYBE_UNUSED,
                            MPI_Comm comm MAYBE_UNUSED) {

}

int MPI_Scan(CONST void* sendbuf,
	     void* recvbuf,
	     int count,
	     MPI_Datatype datatype,
	     MPI_Op op,
	     MPI_Comm comm) {
  FUNCTION_ENTRY;
  MPI_Scan_prolog(sendbuf, recvbuf, count, datatype, op, comm);
  int ret = MPI_Scan_core(sendbuf, recvbuf, count, datatype, op, comm);
  MPI_Scan_epilog(sendbuf, recvbuf, count, datatype, op, comm);
  FUNCTION_EXIT;
  return ret;
}

void mpif_scan_(void* sbuf,
		void* rbuf,
		int* count,
		MPI_Fint* d,
		MPI_Fint* op,
		MPI_Fint* c,
		int* error) {
  FUNCTION_ENTRY_("mpi_scan_");
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Op c_op = MPI_Op_f2c(*op);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);

  MPI_Scan_prolog(sbuf, rbuf, *count, c_type, c_op, c_comm);
  *error = MPI_Scan_core(sbuf, rbuf, *count, c_type, c_op, c_comm);
  MPI_Scan_epilog(sbuf, rbuf, *count, c_type, c_op, c_comm);
  FUNCTION_EXIT_("mpi_scan_");
}
