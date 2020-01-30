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

static void MPI_Alltoallv_prolog(CONST void* sendbuf   MAYBE_UNUSED,
                                 CONST int* sendcnts   MAYBE_UNUSED,
                                 CONST int* sdispls    MAYBE_UNUSED,
                                 MPI_Datatype sendtype MAYBE_UNUSED,
                                 void* recvbuf         MAYBE_UNUSED,
                                 CONST int* recvcnts   MAYBE_UNUSED,
                                 CONST int* rdispls    MAYBE_UNUSED,
                                 MPI_Datatype recvtype MAYBE_UNUSED,
                                 MPI_Comm comm MAYBE_UNUSED) {

}

static int MPI_Alltoallv_core(CONST void* sendbuf, CONST int* sendcnts,
                              CONST int* sdispls, MPI_Datatype sendtype,
                              void* recvbuf, CONST int* recvcnts,
                              CONST int* rdispls, MPI_Datatype recvtype,
                              MPI_Comm comm) {
  return libMPI_Alltoallv(sendbuf, sendcnts, sdispls, sendtype, recvbuf,
                          recvcnts, rdispls, recvtype, comm);
}


static void MPI_Alltoallv_epilog(CONST void* sendbuf   MAYBE_UNUSED,
                                 CONST int* sendcnts   MAYBE_UNUSED,
                                 CONST int* sdispls    MAYBE_UNUSED,
                                 MPI_Datatype sendtype MAYBE_UNUSED,
                                 void* recvbuf         MAYBE_UNUSED,
                                 CONST int* recvcnts   MAYBE_UNUSED,
                                 CONST int* rdispls    MAYBE_UNUSED,
                                 MPI_Datatype recvtype MAYBE_UNUSED,
                                 MPI_Comm comm MAYBE_UNUSED) {
}

int MPI_Alltoallv(CONST void* sendbuf, CONST int* sendcnts, CONST int* sdispls,
                  MPI_Datatype sendtype, void* recvbuf, CONST int* recvcnts,
                  CONST int* rdispls, MPI_Datatype recvtype, MPI_Comm comm) {
  FUNCTION_ENTRY;

  MPI_Alltoallv_prolog(sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts,
                       rdispls, recvtype, comm);
  int ret = MPI_Alltoallv_core(sendbuf, sendcnts, sdispls, sendtype, recvbuf,
                               recvcnts, rdispls, recvtype, comm);
  MPI_Alltoallv_epilog(sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts,
                       rdispls, recvtype, comm);
  FUNCTION_EXIT;
  return ret;
}

void mpif_alltoallv_(void* sbuf, int* scount, int* sdispls, MPI_Fint* sd,
                     void* rbuf, int* rcount, int* rdispls, MPI_Fint* rd,
                     MPI_Fint* c, int* error) {
  FUNCTION_ENTRY_("mpi_alltoallv_");
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  void* c_sbuf = CHECK_MPI_IN_PLACE(sbuf);
  void* c_rbuf = CHECK_MPI_IN_PLACE(rbuf);

  MPI_Alltoallv_prolog(c_sbuf, scount, sdispls, c_stype, c_rbuf, rcount, rdispls,
                       c_rtype, c_comm);
  *error = MPI_Alltoallv_core(c_sbuf, scount, sdispls, c_stype, c_rbuf, rcount,
                              rdispls, c_rtype, c_comm);
  MPI_Alltoallv_epilog(c_sbuf, scount, sdispls, c_stype, c_rbuf, rcount, rdispls,
                       c_rtype, c_comm);
  FUNCTION_EXIT_("mpi_alltoallv_");
}
