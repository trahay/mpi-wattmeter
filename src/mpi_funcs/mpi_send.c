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

static void MPI_Send_prolog(CONST void* buf  MAYBE_UNUSED,
                            int count MAYBE_UNUSED,
                            MPI_Datatype datatype MAYBE_UNUSED,
                            int dest MAYBE_UNUSED,
                            int tag MAYBE_UNUSED,
                            MPI_Comm comm MAYBE_UNUSED) {
}

static int MPI_Send_core(CONST void* buf,
			 int count,
			 MPI_Datatype datatype,
                         int dest,
			 int tag,
			 MPI_Comm comm) {
  return libMPI_Send(buf, count, datatype, dest, tag, comm);
}


static void MPI_Send_epilog(CONST void* buf  MAYBE_UNUSED,
                            int count MAYBE_UNUSED,
                            MPI_Datatype datatype MAYBE_UNUSED,
                            int dest MAYBE_UNUSED,
                            int tag MAYBE_UNUSED,
                            MPI_Comm comm MAYBE_UNUSED) {

}

int MPI_Send(CONST void* buf,
	     int count,
	     MPI_Datatype datatype,
	     int dest,
	     int tag,
	     MPI_Comm comm) {
  FUNCTION_ENTRY;
  MPI_Send_prolog(buf, count, datatype, dest, tag, comm);
  int ret = MPI_Send_core(buf, count, datatype, dest, tag, comm);
  MPI_Send_epilog(buf, count, datatype, dest, tag, comm);
  FUNCTION_EXIT;
  return ret;
}

void mpif_send_(void* buf,
		int* count,
		MPI_Fint* d,
		int* dest,
		int* tag,
		MPI_Fint* c,
		int* error) {
  FUNCTION_ENTRY_("mpi_send_");
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Send_prolog(buf, *count, c_type, *dest, *tag, c_comm);
  *error = MPI_Send_core(buf, *count, c_type, *dest, *tag, c_comm);
  MPI_Send_epilog(buf, *count, c_type, *dest, *tag, c_comm);
  FUNCTION_EXIT_("mpi_send_");
}
