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

static void MPI_Ialltoallv_prolog(CONST void* sendbuf  MAYBE_UNUSED,
                                  CONST int* sendcnts  MAYBE_UNUSED,
                                  CONST int* sdispls  MAYBE_UNUSED,
                                  MPI_Datatype sendtype MAYBE_UNUSED,
                                  void* recvbuf  MAYBE_UNUSED,
                                  CONST int* recvcnts  MAYBE_UNUSED,
                                  CONST int* rdispls  MAYBE_UNUSED,
                                  MPI_Datatype recvtype MAYBE_UNUSED,
                                  MPI_Comm comm MAYBE_UNUSED,
                                  MPI_Request* r MAYBE_UNUSED) {
}

static int MPI_Ialltoallv_core(CONST void* sendbuf,
			       CONST int* sendcnts,
			       CONST int* sdispls,
			       MPI_Datatype sendtype,
                               void* recvbuf,
			       CONST int* recvcnts,
			       CONST int* rdispls,
			       MPI_Datatype recvtype,
                               MPI_Comm comm,
                               MPI_Request* r) {
  return libMPI_Ialltoallv(sendbuf, sendcnts, sdispls, sendtype, recvbuf, recvcnts, rdispls, recvtype, comm, r);
}

static void MPI_Ialltoallv_epilog(CONST void* sendbuf  MAYBE_UNUSED,
                                  CONST int* sendcnts  MAYBE_UNUSED,
                                  CONST int* sdispls  MAYBE_UNUSED,
                                  MPI_Datatype sendtype MAYBE_UNUSED,
                                  void* recvbuf  MAYBE_UNUSED,
                                  CONST int* recvcnts  MAYBE_UNUSED,
                                  CONST int* rdispls  MAYBE_UNUSED,
                                  MPI_Datatype recvtype MAYBE_UNUSED,
                                  MPI_Comm comm MAYBE_UNUSED,
                                  MPI_Request* r MAYBE_UNUSED) {

}

int MPI_Ialltoallv(CONST void* sendbuf,
		   CONST int* sendcnts,
		   CONST int* sdispls,
		   MPI_Datatype sendtype,
                   void* recvbuf,
		   CONST int* recvcnts,
		   CONST int* rdispls,
		   MPI_Datatype recvtype,
                   MPI_Comm comm,
		   MPI_Request* r) {
  FUNCTION_ENTRY;

  MPI_Ialltoallv_prolog(sendbuf, sendcnts, sdispls, sendtype,
			recvbuf, recvcnts, rdispls, recvtype, comm, r);
  int ret = MPI_Ialltoallv_core(sendbuf, sendcnts, sdispls, sendtype,
				recvbuf, recvcnts, rdispls, recvtype, comm, r);
  MPI_Ialltoallv_epilog(sendbuf, sendcnts, sdispls, sendtype,
			recvbuf, recvcnts, rdispls, recvtype, comm, r);
  FUNCTION_EXIT;
  return ret;
}

void mpif_ialltoallv_(void* sbuf,
		      int* scount,
		      int* sdispls,
		      MPI_Fint* sd,
                      void* rbuf,
		      int* rcount,
		      int* rdispls,
		      MPI_Fint* rd,
                      MPI_Fint* c,
		      MPI_Fint* r,
		      int* error) {
  FUNCTION_ENTRY_("mpi_ialltoallv_");
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Request c_req = MPI_Request_f2c(*r);

  MPI_Ialltoallv_prolog(sbuf, scount, sdispls, c_stype,
			rbuf, rcount, rdispls, c_rtype, c_comm, r);
  *error = MPI_Ialltoallv_core(sbuf, scount, sdispls, c_stype,
			       rbuf, rcount, rdispls, c_rtype, c_comm, &c_req);
  *r = MPI_Request_c2f(c_req);
  MPI_Ialltoallv_epilog(sbuf, scount, sdispls, c_stype,
			rbuf, rcount, rdispls, c_rtype, c_comm, r);
  FUNCTION_EXIT_("mpi_ialltoallv_");
}

#endif
