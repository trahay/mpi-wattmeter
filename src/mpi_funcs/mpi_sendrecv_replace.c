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

static void MPI_Sendrecv_replace_prolog(void* buf  MAYBE_UNUSED,
                                        int count MAYBE_UNUSED,
                                        MPI_Datatype type MAYBE_UNUSED,
                                        int dest MAYBE_UNUSED,
                                        int sendtag MAYBE_UNUSED,
                                        int src MAYBE_UNUSED,
                                        int recvtag MAYBE_UNUSED,
                                        MPI_Comm comm MAYBE_UNUSED,
                                        MPI_Status* status MAYBE_UNUSED ) {

}

static int MPI_Sendrecv_replace_core(void* buf,
				     int count,
				     MPI_Datatype type,
				     int dest,
				     int sendtag,
				     int src,
				     int recvtag,
				     MPI_Comm comm,
                                     MPI_Status* status) {
  return libMPI_Sendrecv_replace(buf, count, type, dest, sendtag, src, recvtag,
                                 comm, status);
}


static void MPI_Sendrecv_replace_epilog(void* buf  MAYBE_UNUSED,
                                        int count MAYBE_UNUSED,
                                        MPI_Datatype type MAYBE_UNUSED,
                                        int dest MAYBE_UNUSED,
                                        int sendtag MAYBE_UNUSED,
                                        int src MAYBE_UNUSED,
                                        int recvtag MAYBE_UNUSED,
                                        MPI_Comm comm MAYBE_UNUSED,
                                        MPI_Status* status MAYBE_UNUSED) {

}

int MPI_Sendrecv_replace(void* buf,
			 int count,
			 MPI_Datatype type,
			 int dest,
			 int sendtag,
			 int src,
			 int recvtag,
			 MPI_Comm comm,
                         MPI_Status* status) {
  FUNCTION_ENTRY;
  MPI_Sendrecv_replace_prolog(buf, count, type, dest, sendtag, src, recvtag,
                              comm, status);
  int ret = MPI_Sendrecv_replace_core(buf, count, type, dest, sendtag, src,
                                      recvtag, comm, status);
  MPI_Sendrecv_replace_epilog(buf, count, type, dest, sendtag, src, recvtag,
                              comm, status);
  FUNCTION_EXIT;
  return ret;
}

void mpif_sendrecv_replace_(void* buf,
			    int* count,
			    MPI_Fint* type,
			    int* dest,
			    int* sendtag,
			    int* src,
			    int* recvtag,
			    MPI_Fint* comm,
			    MPI_Status* status,
			    int* error) {
  FUNCTION_ENTRY_("mpi_sendrecv_replace_");
  MPI_Comm c_comm = MPI_Comm_f2c(*comm);
  MPI_Datatype c_type = MPI_Type_f2c(*type);

  MPI_Sendrecv_replace_prolog(buf, *count, c_type, *dest, *sendtag, *src,
                              *recvtag, c_comm, status);
  *error = MPI_Sendrecv_replace_core(buf, *count, c_type, *dest, *sendtag, *src,
                                     *recvtag, c_comm, status);
  MPI_Sendrecv_replace_epilog(buf, *count, c_type, *dest, *sendtag, *src,
                              *recvtag, c_comm, status);
  FUNCTION_EXIT_("mpi_sendrecv_replace_");
}
