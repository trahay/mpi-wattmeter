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

static void MPI_Sendrecv_prolog(CONST void* sendbuf  MAYBE_UNUSED,
                                int sendcount MAYBE_UNUSED,
                                MPI_Datatype sendtype MAYBE_UNUSED,
                                int dest MAYBE_UNUSED,
                                int sendtag MAYBE_UNUSED,
                                void* recvbuf  MAYBE_UNUSED,
                                int recvcount MAYBE_UNUSED,
                                MPI_Datatype recvtype MAYBE_UNUSED,
                                int src MAYBE_UNUSED,
                                int recvtag MAYBE_UNUSED,
                                MPI_Comm comm MAYBE_UNUSED,
                                MPI_Status* status MAYBE_UNUSED) {

}

static int MPI_Sendrecv_core(CONST void* sendbuf,
			     int sendcount,
			     MPI_Datatype sendtype,
			     int dest,
			     int sendtag,
			     void* recvbuf,
			     int recvcount,
			     MPI_Datatype recvtype,
			     int src,
			     int recvtag,
			     MPI_Comm comm,
			     MPI_Status* status) {
  return libMPI_Sendrecv(sendbuf, sendcount, sendtype, dest, sendtag, recvbuf,
                         recvcount, recvtype, src, recvtag, comm, status);
}


static void MPI_Sendrecv_epilog(CONST void* sendbuf  MAYBE_UNUSED,
                                int sendcount MAYBE_UNUSED,
                                MPI_Datatype sendtype MAYBE_UNUSED,
                                int dest MAYBE_UNUSED,
                                int sendtag MAYBE_UNUSED,
                                void* recvbuf  MAYBE_UNUSED,
                                int recvcount MAYBE_UNUSED,
                                MPI_Datatype recvtype MAYBE_UNUSED,
                                int src MAYBE_UNUSED,
                                int recvtag MAYBE_UNUSED,
                                MPI_Comm comm MAYBE_UNUSED,
                                MPI_Status* status MAYBE_UNUSED ) {

}

int MPI_Sendrecv(CONST void* sendbuf,
		 int sendcount,
		 MPI_Datatype sendtype,
		 int dest,
		 int sendtag,
		 void* recvbuf,
		 int recvcount,
		 MPI_Datatype recvtype,
		 int src,
		 int recvtag,
		 MPI_Comm comm,
                 MPI_Status* status) {
  FUNCTION_ENTRY;

  MPI_Sendrecv_prolog(sendbuf, sendcount, sendtype, dest, sendtag, recvbuf,
                      recvcount, recvtype, src, recvtag, comm, status);
  int ret = MPI_Sendrecv_core(sendbuf, sendcount, sendtype, dest, sendtag,
                              recvbuf, recvcount, recvtype, src, recvtag, comm,
                              status);

  MPI_Sendrecv_epilog(sendbuf, sendcount, sendtype, dest, sendtag, recvbuf,
                      recvcount, recvtype, src, recvtag, comm, status);
  FUNCTION_EXIT;
  return ret;
}

void mpif_sendrecv_(void* sendbuf,
		    int* sendcount,
		    MPI_Fint* sendtype,
		    int* dest,
		    int* sendtag,
		    void* recvbuf,
		    int* recvcount,
		    MPI_Fint* recvtype,
		    int* src,
		    int* recvtag,
		    MPI_Fint* comm,
		    MPI_Status* status,
		    int* error) {
  FUNCTION_ENTRY_("mpi_sendrecv_");
  MPI_Comm c_comm = MPI_Comm_f2c(*comm);
  MPI_Datatype c_stype = MPI_Type_f2c(*sendtype);
  MPI_Datatype c_rtype = MPI_Type_f2c(*recvtype);

  MPI_Sendrecv_prolog(sendbuf, *sendcount, c_stype, *dest, *sendtag, recvbuf,
                      *recvcount, c_rtype, *src, *recvtag, c_comm, status);
  *error = MPI_Sendrecv_core(sendbuf, *sendcount, c_stype, *dest, *sendtag,
                             recvbuf, *recvcount, c_rtype, *src, *recvtag,
                             c_comm, status);
  MPI_Sendrecv_epilog(sendbuf, *sendcount, c_stype, *dest, *sendtag, recvbuf,
                      *recvcount, c_rtype, *src, *recvtag, c_comm, status);
  FUNCTION_EXIT_("mpi_sendrecv_");
}
