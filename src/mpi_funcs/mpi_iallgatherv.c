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

static void MPI_Iallgatherv_prolog(CONST void* sendbuf  MAYBE_UNUSED,
                                   int sendcount  MAYBE_UNUSED,
                                   MPI_Datatype sendtype  MAYBE_UNUSED,
                                   void* recvbuf  MAYBE_UNUSED,
                                   CONST int* recvcounts  MAYBE_UNUSED,
                                   CONST int* displs  MAYBE_UNUSED,
                                   MPI_Datatype recvtype  MAYBE_UNUSED,
                                   MPI_Comm comm  MAYBE_UNUSED,
                                   MPI_Request* r MAYBE_UNUSED) {
}

static int MPI_Iallgatherv_core(CONST void* sendbuf ,
                                int sendcount,
                                MPI_Datatype sendtype,
                                void* recvbuf ,
                                CONST int* recvcounts ,
                                CONST int* displs ,
                                MPI_Datatype recvtype,
                                MPI_Comm comm,
                                MPI_Request* r) {
  return libMPI_Iallgatherv(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, r);
}

static void MPI_Iallgatherv_epilog(CONST void* sendbuf  MAYBE_UNUSED,
                                   int sendcount  MAYBE_UNUSED,
                                   MPI_Datatype sendtype  MAYBE_UNUSED,
                                   void* recvbuf  MAYBE_UNUSED,
                                   CONST int* recvcounts  MAYBE_UNUSED,
                                   CONST int* displs  MAYBE_UNUSED,
                                   MPI_Datatype recvtype  MAYBE_UNUSED,
                                   MPI_Comm comm  MAYBE_UNUSED,
                                   MPI_Request* r MAYBE_UNUSED) {

}

int MPI_Iallgatherv(CONST void* sendbuf,
		    int sendcount,
		    MPI_Datatype sendtype,
                    void* recvbuf,
		    CONST int* recvcounts,
		    CONST int* displs,
                    MPI_Datatype recvtype,
		    MPI_Comm comm,
		    MPI_Request* r) {
  FUNCTION_ENTRY;
  MPI_Iallgatherv_prolog(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, r);
  int ret = MPI_Iallgatherv_core(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, r);
  MPI_Iallgatherv_epilog(sendbuf, sendcount, sendtype, recvbuf, recvcounts, displs, recvtype, comm, r);
  FUNCTION_EXIT;
  return ret;
}

void mpif_iallgatherv_(void* sbuf,
		       int* scount,
		       MPI_Fint* sd,
                       void* rbuf,
		       int* rcount,
		       int* displs,
                       MPI_Fint* rd,
		       MPI_Fint* c,
		       MPI_Fint* r,
		       int* error) {
  FUNCTION_ENTRY_("mpi_iallgatherv_");
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Request c_req = MPI_Request_f2c(*r);

  MPI_Iallgatherv_prolog(sbuf, *scount, c_stype, rbuf, rcount, displs, c_rtype, c_comm, r);
  *error = MPI_Iallgatherv_core(sbuf, *scount, c_stype, rbuf, rcount, displs, c_rtype, c_comm, &c_req);
  *r = MPI_Request_c2f(c_req);
  MPI_Iallgatherv_epilog(sbuf, *scount, c_stype, rbuf, rcount, displs, c_rtype, c_comm, r);
  FUNCTION_EXIT_("mpi_iallgatherv_");
}

#endif
