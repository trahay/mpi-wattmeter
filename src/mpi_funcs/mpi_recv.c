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

static void MPI_Recv_prolog(void* buf  MAYBE_UNUSED,
			    int count MAYBE_UNUSED,
			    MPI_Datatype datatype MAYBE_UNUSED,
			    int source MAYBE_UNUSED,
			    int tag MAYBE_UNUSED,
                            MPI_Comm comm MAYBE_UNUSED,
                            MPI_Status* status MAYBE_UNUSED ) {

}

static int MPI_Recv_core(void* buf,
			 int count,
			 MPI_Datatype datatype,
			 int source,
			 int tag,
			 MPI_Comm comm,
			 MPI_Status* status) {
  return libMPI_Recv(buf, count, datatype, source, tag, comm, status);
}


static void MPI_Recv_epilog(void* buf  MAYBE_UNUSED,
			    int count MAYBE_UNUSED,			    
                            MPI_Datatype datatype MAYBE_UNUSED,
			    int source MAYBE_UNUSED,
			    int tag MAYBE_UNUSED,
                            MPI_Comm comm MAYBE_UNUSED,
                            MPI_Status* status MAYBE_UNUSED) {
}

int MPI_Recv(void* buf,
	     int count,
	     MPI_Datatype datatype,
	     int source,
	     int tag,
	     MPI_Comm comm,
	     MPI_Status* status) {
  FUNCTION_ENTRY;
  MPI_Status ezt_mpi_status;
  if(!status || status == MPI_STATUS_IGNORE) 
    status = &ezt_mpi_status;

  MPI_Recv_prolog(buf, count, datatype, source, tag, comm, status);
  int ret = MPI_Recv_core(buf, count, datatype, source, tag, comm, status);
  MPI_Recv_epilog(buf, count, datatype, source, tag, comm, status);
  
  FUNCTION_EXIT;
  return ret;
}

void mpif_recv_(void* buf,
		int* count,
		MPI_Fint* d,
		int* src,
		int* tag,	
                MPI_Fint* c,
		MPI_Fint* s,
		int* error) {
  FUNCTION_ENTRY_("mpi_recv_");
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Datatype c_type = MPI_Type_f2c(*d);
  MPI_Status c_status;

  MPI_Recv_prolog(buf, *count, c_type, *src, *tag, c_comm, &c_status);
  *error = MPI_Recv_core(buf, *count, c_type, *src, *tag, c_comm, &c_status);
  MPI_Status_c2f(&c_status, s);
  MPI_Recv_epilog(buf, *count, c_type, *src, *tag, c_comm, &c_status);
  FUNCTION_EXIT_("mpi_recv_");
}
