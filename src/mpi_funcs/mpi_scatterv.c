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

static void MPI_Scatterv_prolog(CONST void* sendbuf  MAYBE_UNUSED,
                                CONST int* sendcnts  MAYBE_UNUSED,
                                CONST int* displs  MAYBE_UNUSED,
                                MPI_Datatype sendtype  MAYBE_UNUSED,
                                void* recvbuf  MAYBE_UNUSED,
                                int recvcnt MAYBE_UNUSED,
                                MPI_Datatype recvtype MAYBE_UNUSED,
                                int root MAYBE_UNUSED,
                                MPI_Comm comm MAYBE_UNUSED) {
}

static int MPI_Scatterv_core(CONST void* sendbuf,
                             CONST int* sendcnts,
                             CONST int* displs,
                             MPI_Datatype sendtype,
                             void* recvbuf,
                             int recvcnt,
                             MPI_Datatype recvtype,
                             int root,
                             MPI_Comm comm) {
  return libMPI_Scatterv(sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt,
                         recvtype, root, comm);
}


static void MPI_Scatterv_epilog(CONST void* sendbuf  MAYBE_UNUSED,
                                CONST int* sendcnts  MAYBE_UNUSED,
                                CONST int* displs  MAYBE_UNUSED,
                                MPI_Datatype sendtype  MAYBE_UNUSED,
                                void* recvbuf  MAYBE_UNUSED,
                                int recvcnt MAYBE_UNUSED,
                                MPI_Datatype recvtype MAYBE_UNUSED,
                                int root MAYBE_UNUSED,
                                MPI_Comm comm MAYBE_UNUSED) {
}

int MPI_Scatterv(CONST void* sendbuf,
                 CONST int* sendcnts,
                 CONST int* displs,
                 MPI_Datatype sendtype,
                 void* recvbuf,
                 int recvcnt,
                 MPI_Datatype recvtype,
                 int root,
                 MPI_Comm comm) {
  FUNCTION_ENTRY;
  MPI_Scatterv_prolog(sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt,
                      recvtype, root, comm);
  int ret = MPI_Scatterv_core(sendbuf, sendcnts, displs, sendtype, recvbuf,
                              recvcnt, recvtype, root, comm);
  MPI_Scatterv_epilog(sendbuf, sendcnts, displs, sendtype, recvbuf, recvcnt,
                      recvtype, root, comm);
  FUNCTION_EXIT;
  return ret;
}

void mpif_scatterv_(void* sbuf,
		    int* scount,
		    int* displs,
		    MPI_Fint* sd,
		    void* rbuf,
		    int* rcount,
		    MPI_Fint* rd,
		    int* root,
		    MPI_Fint* c,
		    int* error) {
  FUNCTION_ENTRY_("mpi_scatterv_");
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  void* c_sbuf = CHECK_MPI_IN_PLACE(sbuf);
  void* c_rbuf = CHECK_MPI_IN_PLACE(rbuf);

  MPI_Scatterv_prolog(c_sbuf, scount, displs, c_stype, c_rbuf, *rcount, c_rtype,
                      *root, c_comm);

  *error = MPI_Scatterv_core(c_sbuf, scount, displs, c_stype, c_rbuf, *rcount,
                             c_rtype, *root, c_comm);

  MPI_Scatterv_epilog(c_sbuf, scount, displs, c_stype, c_rbuf, *rcount, c_rtype,
                      *root, c_comm);
  FUNCTION_EXIT_("mpi_scatterv_");
}
