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

static void MPI_Iscatter_prolog(CONST void* sendbuf  MAYBE_UNUSED,
                                int sendcnt  MAYBE_UNUSED,
                                MPI_Datatype sendtype  MAYBE_UNUSED,
                                void* recvbuf  MAYBE_UNUSED,
                                int recvcnt  MAYBE_UNUSED,
                                MPI_Datatype recvtype  MAYBE_UNUSED,
                                int root MAYBE_UNUSED,
                                MPI_Comm comm MAYBE_UNUSED,
                                MPI_Request* r MAYBE_UNUSED) {
}

static int MPI_Iscatter_core(CONST void* sendbuf,
			     int sendcnt,
			     MPI_Datatype sendtype,
			     void* recvbuf,
			     int recvcnt,
                             MPI_Datatype recvtype,
			     int root,
			     MPI_Comm comm,
			     MPI_Request* r) {
  return libMPI_Iscatter(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm, r);
}

static void MPI_Iscatter_epilog(CONST void* sendbuf  MAYBE_UNUSED,
                                int sendcnt  MAYBE_UNUSED,
                                MPI_Datatype sendtype  MAYBE_UNUSED,
                                void* recvbuf  MAYBE_UNUSED,
                                int recvcnt  MAYBE_UNUSED,
                                MPI_Datatype recvtype  MAYBE_UNUSED,
                                int root MAYBE_UNUSED,
                                MPI_Comm comm MAYBE_UNUSED,
                                MPI_Request* r MAYBE_UNUSED) {

}

int MPI_Iscatter(CONST void* sendbuf,
		 int sendcnt,
		 MPI_Datatype sendtype,
		 void* recvbuf,
		 int recvcnt,
                 MPI_Datatype recvtype,
		 int root,
		 MPI_Comm comm,
		 MPI_Request* r) {
  FUNCTION_ENTRY;

  MPI_Iscatter_prolog(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm, r);
  int ret = MPI_Iscatter_core(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm, r);
  MPI_Iscatter_epilog(sendbuf, sendcnt, sendtype, recvbuf, recvcnt, recvtype, root, comm, r);
  FUNCTION_EXIT;
  return ret;
}

void mpif_iscatter_(void* sbuf,
		    int* scount,
		    MPI_Fint* sd,
                    void* rbuf,
		    int* rcount,
		    MPI_Fint* rd,
                    int* root,
		    MPI_Fint* c,
		    MPI_Fint* r,
		    int* error) {
  FUNCTION_ENTRY_("mpi_iscatter_");
  MPI_Datatype c_stype = MPI_Type_f2c(*sd);
  MPI_Datatype c_rtype = MPI_Type_f2c(*rd);
  MPI_Comm c_comm = MPI_Comm_f2c(*c);
  MPI_Request c_req = MPI_Request_f2c(*r);

  MPI_Iscatter_prolog(sbuf, *scount, c_stype,
                      rbuf, *rcount, c_rtype,
                      *root, c_comm, r);
  *error = MPI_Iscatter_core(sbuf, *scount, c_stype,
                             rbuf, *rcount, c_rtype,
                             *root, c_comm, &c_req);

  *r = MPI_Request_c2f(c_req);
  MPI_Iscatter_epilog(sbuf, *scount, c_stype,
                      rbuf, *rcount, c_rtype,
                      *root, c_comm, r);
  FUNCTION_EXIT_("mpi_iscatter_");
}

#endif
