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

static void MPI_Bcast_prolog(void* buffer MAYBE_UNUSED,
			     int count MAYBE_UNUSED,
                             MPI_Datatype datatype MAYBE_UNUSED,
			     int root MAYBE_UNUSED,
			     MPI_Comm comm MAYBE_UNUSED) {

}

static int MPI_Bcast_core(void* buffer,
			  int count,
			  MPI_Datatype datatype,
                          int root,
			  MPI_Comm comm) {
  return libMPI_Bcast(buffer, count, datatype, root, comm);
}


static void MPI_Bcast_epilog(void* buffer MAYBE_UNUSED,
			     int count MAYBE_UNUSED,
                             MPI_Datatype datatype MAYBE_UNUSED,
			     int root MAYBE_UNUSED,
			     MPI_Comm comm MAYBE_UNUSED) {

}

int MPI_Bcast(void* buffer,
	      int count,
	      MPI_Datatype datatype,
	      int root,
              MPI_Comm comm) {
  FUNCTION_ENTRY;
  MPI_Bcast_prolog(buffer, count, datatype, root, comm);
  int ret = MPI_Bcast_core(buffer, count, datatype, root, comm);
  MPI_Bcast_epilog(buffer, count, datatype, root, comm);
  FUNCTION_EXIT;
  return ret;
}

void mpif_bcast_(void* buf,
		 int* count,
		 MPI_Fint* d,
		 int* root,
		 MPI_Fint* c,
                 int* error) {
  FUNCTION_ENTRY_("mpi_bcast_");
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Bcast_prolog(buf, *count, c_type, *root, c_comm);
  *error = MPI_Bcast_core(buf, *count, c_type, *root, c_comm);
  MPI_Bcast_epilog(buf, *count, c_type, *root, c_comm);
  FUNCTION_EXIT_("mpi_bcast_");
}
