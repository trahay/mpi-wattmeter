/* -*- C-File-style: "GNU" -*- */
/*
 * Copyright (C) Telecom SudParis
 * See COPYING in top-level directory.
 */

#ifndef _REENTRANT
#define _REENTRANT
#endif
#include "mpii.h"

#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <sys/timeb.h>
#include <unistd.h>
#include <errno.h>

#include <mpi.h>

struct mpii_info mpii_infos; /* information on the local process */

/* pointers to actual MPI functions (C version)  */
int (*libMPI_Init)(int*, char***);
int (*libMPI_Init_thread)(int*, char***, int, int*);
int (*libMPI_Comm_size)(MPI_Comm, int*);
int (*libMPI_Comm_rank)(MPI_Comm, int*);
int (*libMPI_Comm_get_parent)(MPI_Comm* parent) = NULL;
int (*libMPI_Finalize)(void);
int (*libMPI_Initialized)(int*);
int (*libMPI_Abort)(MPI_Comm, int);
int (*libMPI_Type_size)(MPI_Datatype datatype, int* size);

int (*libMPI_Cancel)(MPI_Request*);

int (*libMPI_Comm_disconnect)(MPI_Comm* comm);
int (*libMPI_Comm_free)(MPI_Comm* comm);

int (*libMPI_Comm_create)(MPI_Comm comm, MPI_Group group, MPI_Comm* newcomm);
int (*libMPI_Comm_create_group)(MPI_Comm comm, MPI_Group group, int tag, MPI_Comm* newcomm);

int (*libMPI_Comm_split)(MPI_Comm comm, int color, int key, MPI_Comm* newcomm);
int (*libMPI_Comm_dup)(MPI_Comm comm, MPI_Comm* newcomm);
int (*libMPI_Comm_dup_with_info)(MPI_Comm comm, MPI_Info info, MPI_Comm* newcomm);

int (*libMPI_Comm_split_type)(MPI_Comm comm, int split_type, int key, MPI_Info info,
                              MPI_Comm* newcomm);

int (*libMPI_Intercomm_create)(MPI_Comm local_comm, int local_leader,
                               MPI_Comm peer_comm, int remote_leader, int tag,
                               MPI_Comm* newintercomm);
int (*libMPI_Intercomm_merge)(MPI_Comm intercomm, int high,
                              MPI_Comm* newintracomm);
int (*libMPI_Cart_sub)(MPI_Comm old_comm, CONST int* belongs,
                       MPI_Comm* new_comm);
int (*libMPI_Cart_create)(MPI_Comm comm_old, int ndims, CONST int* dims,
                          CONST int* periods, int reorder, MPI_Comm* comm_cart);
int (*libMPI_Graph_create)(MPI_Comm comm_old, int nnodes, CONST int* index,
                           CONST int* edges, int reorder, MPI_Comm* comm_graph);

int (*libMPI_Dist_graph_create_adjacent)(MPI_Comm comm_old,
                                         int indegree,
                                         CONST int sources[],
                                         CONST int sourceweights[],
                                         int outdegree,
                                         CONST int destinations[],
                                         CONST int destweights[],
                                         MPI_Info info,
                                         int reorder,
                                         MPI_Comm* comm_dist_graph);
int (*libMPI_Dist_graph_create)(MPI_Comm comm_old, int n, CONST int sources[],
                                CONST int degrees[], CONST int destinations[],
                                CONST int weights[], MPI_Info info, int reorder,
                                MPI_Comm* comm_dist_graph);

int (*libMPI_Send)(CONST void* buf, int count, MPI_Datatype datatype, int dest,
                   int tag, MPI_Comm comm);
int (*libMPI_Recv)(void* buf, int count, MPI_Datatype datatype, int source,
                   int tag, MPI_Comm comm, MPI_Status* status);

int (*libMPI_Bsend)(CONST void*, int, MPI_Datatype, int, int, MPI_Comm);
int (*libMPI_Ssend)(CONST void*, int, MPI_Datatype, int, int, MPI_Comm);
int (*libMPI_Rsend)(CONST void*, int, MPI_Datatype, int, int, MPI_Comm);
int (*libMPI_Isend)(CONST void*, int, MPI_Datatype, int, int, MPI_Comm,
                    MPI_Request*);
int (*libMPI_Ibsend)(CONST void*, int, MPI_Datatype, int, int, MPI_Comm,
                     MPI_Request*);
int (*libMPI_Issend)(CONST void*, int, MPI_Datatype, int, int, MPI_Comm,
                     MPI_Request*);
int (*libMPI_Irsend)(CONST void*, int, MPI_Datatype, int, int, MPI_Comm,
                     MPI_Request*);
int (*libMPI_Irecv)(void*, int, MPI_Datatype, int, int, MPI_Comm, MPI_Request*);

int (*libMPI_Sendrecv)(CONST void*, int, MPI_Datatype, int, int, void*, int,
                       MPI_Datatype, int, int, MPI_Comm, MPI_Status*);
int (*libMPI_Sendrecv_replace)(void*, int, MPI_Datatype, int, int, int, int,
                               MPI_Comm, MPI_Status*);

int (*libMPI_Send_init)(CONST void*, int, MPI_Datatype, int, int, MPI_Comm,
                        MPI_Request*);
int (*libMPI_Bsend_init)(CONST void*, int, MPI_Datatype, int, int, MPI_Comm,
                         MPI_Request*);
int (*libMPI_Ssend_init)(CONST void*, int, MPI_Datatype, int, int, MPI_Comm,
                         MPI_Request*);
int (*libMPI_Rsend_init)(CONST void*, int, MPI_Datatype, int, int, MPI_Comm,
                         MPI_Request*);
int (*libMPI_Recv_init)(void*, int, MPI_Datatype, int, int, MPI_Comm,
                        MPI_Request*);
int (*libMPI_Start)(MPI_Request*);
int (*libMPI_Startall)(int, MPI_Request*);

int (*libMPI_Wait)(MPI_Request*, MPI_Status*);
int (*libMPI_Test)(MPI_Request*, int*, MPI_Status*);
int (*libMPI_Waitany)(int, MPI_Request*, int*, MPI_Status*);
int (*libMPI_Testany)(int, MPI_Request*, int*, int*, MPI_Status*);
int (*libMPI_Waitall)(int, MPI_Request*, MPI_Status*);
int (*libMPI_Testall)(int, MPI_Request*, int*, MPI_Status*);
int (*libMPI_Waitsome)(int, MPI_Request*, int*, int*, MPI_Status*);
int (*libMPI_Testsome)(int, MPI_Request*, int*, int*, MPI_Status*);

int (*libMPI_Probe)(int source, int tag, MPI_Comm comm, MPI_Status* status);
int (*libMPI_Iprobe)(int source, int tag, MPI_Comm comm, int* flag,
                     MPI_Status* status);

int (*libMPI_Barrier)(MPI_Comm);
int (*libMPI_Bcast)(void*, int, MPI_Datatype, int, MPI_Comm);
int (*libMPI_Gather)(CONST void*, int, MPI_Datatype, void*, int, MPI_Datatype,
                     int, MPI_Comm);
int (*libMPI_Gatherv)(CONST void*, int, MPI_Datatype, void*, CONST int*,
                      CONST int*, MPI_Datatype, int, MPI_Comm);
int (*libMPI_Scatter)(CONST void*, int, MPI_Datatype, void*, int, MPI_Datatype,
                      int, MPI_Comm);
int (*libMPI_Scatterv)(CONST void*, CONST int*, CONST int*, MPI_Datatype,
                       void*, int, MPI_Datatype, int, MPI_Comm);
int (*libMPI_Allgather)(CONST void*, int, MPI_Datatype, void*, int,
                        MPI_Datatype, MPI_Comm);
int (*libMPI_Allgatherv)(CONST void*, int, MPI_Datatype, void*, CONST int*,
                         CONST int*, MPI_Datatype, MPI_Comm);
int (*libMPI_Alltoall)(CONST void*, int, MPI_Datatype, void*, int, MPI_Datatype,
                       MPI_Comm);
int (*libMPI_Alltoallv)(CONST void*, CONST int*, CONST int*, MPI_Datatype,
                        void*, CONST int*, CONST int*, MPI_Datatype, MPI_Comm);
int (*libMPI_Reduce)(CONST void*, void*, int, MPI_Datatype, MPI_Op, int,
                     MPI_Comm);
int (*libMPI_Allreduce)(CONST void*, void*, int, MPI_Datatype, MPI_Op, MPI_Comm);
int (*libMPI_Reduce_scatter)(CONST void*, void*, CONST int*, MPI_Datatype,
                             MPI_Op, MPI_Comm);
int (*libMPI_Scan)(CONST void*, void*, int, MPI_Datatype, MPI_Op, MPI_Comm);

/* non-blocking collective (available since MPI 3.0) */
#ifdef USE_MPI3
int (*libMPI_Ibarrier)(MPI_Comm, MPI_Request*);
int (*libMPI_Ibcast)(void*, int, MPI_Datatype, int, MPI_Comm, MPI_Request*);
int (*libMPI_Igather)(const void*, int, MPI_Datatype, void*, int, MPI_Datatype, int, MPI_Comm, MPI_Request*);
int (*libMPI_Igatherv)(const void*, int, MPI_Datatype, void*, const int*, const int*, MPI_Datatype, int, MPI_Comm, MPI_Request*);
int (*libMPI_Iscatter)(const void*, int, MPI_Datatype, void*, int, MPI_Datatype, int, MPI_Comm, MPI_Request*);
int (*libMPI_Iscatterv)(const void*, const int*, const int*, MPI_Datatype, void*, int, MPI_Datatype, int, MPI_Comm, MPI_Request*);
int (*libMPI_Iallgather)(const void*, int, MPI_Datatype, void*, int, MPI_Datatype, MPI_Comm, MPI_Request*);
int (*libMPI_Iallgatherv)(const void*, int, MPI_Datatype, void*, const int*, const int*, MPI_Datatype, MPI_Comm, MPI_Request*);
int (*libMPI_Ialltoall)(const void*, int, MPI_Datatype, void*, int, MPI_Datatype, MPI_Comm, MPI_Request*);
int (*libMPI_Ialltoallv)(const void*, const int*, const int*, MPI_Datatype, void*, const int*, const int*, MPI_Datatype, MPI_Comm, MPI_Request*);
int (*libMPI_Ireduce)(const void*, void*, int, MPI_Datatype, MPI_Op, int, MPI_Comm, MPI_Request*);
int (*libMPI_Iallreduce)(const void*, void*, int, MPI_Datatype, MPI_Op, MPI_Comm, MPI_Request*);
int (*libMPI_Ireduce_scatter)(const void*, void*, const int*, MPI_Datatype, MPI_Op, MPI_Comm, MPI_Request*);
int (*libMPI_Iscan)(const void*, void*, int, MPI_Datatype, MPI_Op, MPI_Comm, MPI_Request*);
#endif

int (*libMPI_Get)(void*, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
                  MPI_Win);
int (*libMPI_Put)(CONST void*, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
                  MPI_Win);

int (*libMPI_Get)(void*, int, MPI_Datatype, int, MPI_Aint, int, MPI_Datatype,
                  MPI_Win);
int (*libMPI_Put)(CONST void*, int, MPI_Datatype, int, MPI_Aint, int,
                  MPI_Datatype, MPI_Win);

int (*libMPI_Comm_spawn)(CONST char* command, char* argv[], int maxprocs,
                         MPI_Info info, int root, MPI_Comm comm,
                         MPI_Comm* intercomm, int array_of_errcodes[]);

/* fortran bindings */
void (*libmpi_init_)(int* e);
void (*libmpi_init_thread_)(int*, int*, int*);
void (*libmpi_finalize_)(int*);
void (*libmpi_barrier_)(MPI_Comm*, int*);
void (*libmpi_comm_size_)(MPI_Comm*, int*, int*);
void (*libmpi_comm_rank_)(MPI_Comm*, int*, int*);
void (*libmpi_comm_get_parent_)(MPI_Comm*, int*);
void (*libmpi_type_size_)(MPI_Datatype*, int*, int*);

void (*libmpi_cancel_)(MPI_Request*, int*);

int (*libmpi_comm_create_)(int*, int*, int*, int*);
int (*libmpi_comm_create_group_)(int*, int*, int*, int*, int*);

int (*libmpi_comm_split_)(int*, int*, int*, int*, int*);
int (*libmpi_comm_dup_)(int*, int*, int*);
int (*libmpi_comm_dup_with_info_)(int*, int*, int*, int*);

int (*libmpi_comm_split_type_)(int*, int*, int*, int*, int*);

int (*libmpi_intercomm_create_)(int*, int*, int*, int*, int*, int*, int*);
int (*libmpi_intercomm_merge_)(int*, int*, int*, int*);
int (*libmpi_cart_sub_)(int*, int*, int*, int*);
int (*libmpi_cart_create_)(int*, int*, int*, int*, int*, int*, int*);
int (*libmpi_graph_create_)(int*, int*, int*, int*, int*, int*, int*);

int (*libmpi_dist_graph_create_adjacent_)(int*, int*, int*, int*, int*, int*,
                                          int*, int*, int*, int*, int*);

int (*libmpi_dist_graph_create_)(int*, int*, int*, int*, int*, int*, int*, int*, int*, int*);

void (*libmpi_send_)(void*, int*, MPI_Datatype*, int*, int*, int*);
void (*libmpi_recv_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Status*,
                     int*);

void (*libmpi_sendrecv_)(void*, int, MPI_Datatype, int, int, void*, int,
                         MPI_Datatype, int, int, MPI_Comm, MPI_Status*, int*);
void (*libmpi_sendrecv_replace_)(void*, int, MPI_Datatype, int, int, int, int,
                                 MPI_Comm, MPI_Status*, int*);

void (*libmpi_bsend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, int*);
void (*libmpi_ssend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, int*);
void (*libmpi_rsend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*, int*);
void (*libmpi_isend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*,
                      MPI_Request*, int*);
void (*libmpi_ibsend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*,
                       MPI_Request*, int*);
void (*libmpi_issend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*,
                       MPI_Request*, int*);
void (*libmpi_irsend_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*,
                       MPI_Request*, int*);
void (*libmpi_irecv_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*,
                      MPI_Request*, int*);

void (*libmpi_wait_)(MPI_Request*, MPI_Status*, int*);
void (*libmpi_test_)(MPI_Request*, int*, MPI_Status*, int*);
void (*libmpi_waitany_)(int*, MPI_Request*, int*, MPI_Status*, int*);
void (*libmpi_testany_)(int*, MPI_Request*, int*, int*, MPI_Status*, int*);
void (*libmpi_waitall_)(int*, MPI_Request*, MPI_Status*, int*);
void (*libmpi_testall_)(int*, MPI_Request*, int*, MPI_Status*, int*);
void (*libmpi_waitsome_)(int*, MPI_Request*, int*, int*, MPI_Status*, int*);
void (*libmpi_testsome_)(int*, MPI_Request*, int*, int*, MPI_Status*, int*);

void (*libmpi_probe_)(int* source, int* tag, MPI_Comm* comm, MPI_Status* status,
                      int* err);
void (*libmpi_iprobe_)(int* source, int* tag, MPI_Comm* comm, int* flag,
                       MPI_Status* status, int* err);

void (*libmpi_get_)(void*, int*, MPI_Datatype*, int*, MPI_Aint*, int*,
                    MPI_Datatype*, MPI_Win*, int*);
void (*libmpi_put_)(void*, int*, MPI_Datatype*, int*, MPI_Aint*, int*,
                    MPI_Datatype*, MPI_Win*, int*);

void (*libmpi_bcast_)(void*, int*, MPI_Datatype*, int*, MPI_Comm*, int*);
void (*libmpi_gather_)(void*, int*, MPI_Datatype*, void*, int*, MPI_Datatype*,
                       int*, MPI_Comm*, int*);
void (*libmpi_gatherv_)(void*, int*, MPI_Datatype*, void*, int*, int*,
                        MPI_Datatype*, int*, MPI_Comm*);
void (*libmpi_scatter_)(void*, int*, MPI_Datatype*, void*, int*, MPI_Datatype*,
                        int*, MPI_Comm*, int*);
void (*libmpi_scatterv_)(void*, int*, int*, MPI_Datatype*, void*, int*,
                         MPI_Datatype*, int*, MPI_Comm*, int*);
void (*libmpi_allgather_)(void*, int*, MPI_Datatype*, void*, int*,
                          MPI_Datatype*, MPI_Comm*, int*);
void (*libmpi_allgatherv_)(void*, int*, MPI_Datatype*, void*, int*, int*,
                           MPI_Datatype*, MPI_Comm*);
void (*libmpi_alltoall_)(void*, int*, MPI_Datatype*, void*, int*, MPI_Datatype*,
                         MPI_Comm*, int*);
void (*libmpi_alltoallv_)(void*, int*, int*, MPI_Datatype*, void*, int*, int*,
                          MPI_Datatype*, MPI_Comm*, int*);
void (*libmpi_reduce_)(void*, void*, int*, MPI_Datatype*, MPI_Op*, int*,
                       MPI_Comm*, int*);
void (*libmpi_allreduce_)(void*, void*, int*, MPI_Datatype*, MPI_Op*, MPI_Comm*,
                          int*);
void (*libmpi_reduce_scatter_)(void*, void*, int*, MPI_Datatype*, MPI_Op*,
                               MPI_Comm*, int*);
void (*libmpi_scan_)(void*, void*, int*, MPI_Datatype*, MPI_Op*, MPI_Comm*,
                     int*);

#ifdef USE_MPI3
/* non-blocking collective (available since MPI 3.0) */
void (*libmpi_ibarrier_)(MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_ibcast_)(void*, int*, MPI_Fint*, int*, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_igather_)(const void*, int*, MPI_Fint*, void*, int*, MPI_Fint*, int*, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_igatherv_)(const void*, int*, MPI_Fint*, void*, const int*, const int*, MPI_Fint*, int*, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_iscatter_)(const void*, int*, MPI_Fint*, void*, int*, MPI_Fint*, int*, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_iscatterv_)(const void*, const int*, const int*, MPI_Fint*, void*, int*, MPI_Fint*, int*, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_iallgather_)(const void*, int*, MPI_Fint*, void*, int*, MPI_Fint*, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_iallgatherv_)(const void*, int*, MPI_Fint*, void*, const int*, const int*, MPI_Fint*, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_ialltoall_)(const void*, int*, MPI_Fint*, void*, int*, MPI_Fint*, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_ialltoallv_)(const void*, const int*, const int*, MPI_Fint*, void*, const int*, const int*, MPI_Fint*, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_ireduce_)(const void*, void*, int*, MPI_Fint*, MPI_Op, int*, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_iallreduce_)(const void*, void*, int*, MPI_Fint*, MPI_Op, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_ireduce_scatter_)(const void*, void*, const int*, MPI_Fint*, MPI_Op, MPI_Fint*, MPI_Fint*, int*);
void (*libmpi_iscan_)(const void*, void*, int*, MPI_Fint*, MPI_Op, MPI_Fint*, MPI_Fint*, int*);
#endif

void (*libmpi_comm_spawn_)(char* command, char** argv, int* maxprocs,
                           MPI_Info* info, int* root, MPI_Comm* comm,
                           MPI_Comm* intercomm, int* array_of_errcodes,
                           int* error);

void (*libmpi_send_init_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*,
                          MPI_Request*, int*);
void (*libmpi_bsend_init_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*,
                           MPI_Request*, int*);
void (*libmpi_ssend_init_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*,
                           MPI_Request*, int*);
void (*libmpi_rsend_init_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*,
                           MPI_Request*, int*);
void (*libmpi_recv_init_)(void*, int*, MPI_Datatype*, int*, int*, MPI_Comm*,
                          MPI_Request*, int*);
void (*libmpi_start_)(MPI_Request*, int*);
void (*libmpi_startall_)(int*, MPI_Request*, int*);

static int __mpi_init_called = 0;

/* Functions that intercept MPI calls
 * Basically each function create an event this the arguments
 * passed to the function.
 * It then call the actual MPI function (using the appropriate
 * callback) with the same args
 */
int MPI_Comm_spawn(CONST char* command MAYBE_UNUSED,
		   char* argv[] MAYBE_UNUSED,
		   int maxprocs MAYBE_UNUSED,
                   MPI_Info info MAYBE_UNUSED,
		   int root MAYBE_UNUSED,
		   MPI_Comm comm MAYBE_UNUSED,
		   MPI_Comm* intercomm MAYBE_UNUSED,
                   int array_of_errcodes[] MAYBE_UNUSED) {

  /* TODO */
#if 0
  
  /* Instead of running command argv, we have to
   run 'env LD_PRELOAD=xxx command argv'
   Thus, we have to provide a new argv array
   */

  /* retrieve LD_PRELOAD command set by EZTrace */
  char* ld_preload = getenv(LD_PRELOAD_NAME);
  char* ld_preload_str = NULL;
  int ret __attribute__((__unused__));
  ret = asprintf(&ld_preload_str, "%s=%s", LD_PRELOAD_NAME, ld_preload);

  /* count the number of args */
  int argc = 0;
  if (argv != MPI_ARGV_NULL)
    for (argc = 0; argv[argc] != NULL; argc++) {
    }

  /* create a new argv array */
  int new_argc = argc + 3;
  char** new_argv = (char**)malloc(sizeof(char*) * new_argc);

  new_argv[0] = ld_preload_str;
  ret = asprintf(&new_argv[1], "%s", command);
  int i;
  for (i = 0; i < argc; i++)
    new_argv[i + 2] = argv[i];

  new_argv[i + 2] = NULL;

  ret = libMPI_Comm_spawn("env", new_argv, maxprocs, info, root, comm,
                          intercomm, array_of_errcodes);


  /* Here, we shall not free ld_preload, since it may modify the environment of the process ! (man getenv) */
  free(new_argv);
  free(ld_preload_str);

  FUNCTION_ENTRY;
  return ret;
#endif
  return 0;
}

int MPI_Comm_get_parent(MPI_Comm* parent) {
  if (!libMPI_Comm_get_parent) {
    /* MPI_Comm_get_parent was not found. Let's assume the application doesn't use it. */
    *parent = MPI_COMM_NULL;
    return MPI_SUCCESS;
  }
  return libMPI_Comm_get_parent(parent);
}

int MPI_Comm_size(MPI_Comm c, int* s) {
  return libMPI_Comm_size(c, s);
}

int MPI_Comm_rank(MPI_Comm c, int* r) {
  return libMPI_Comm_rank(c, r);
}

int MPI_Type_size(MPI_Datatype datatype, int* size) {
  return libMPI_Type_size(datatype, size);
}

int MPI_Finalize() {
  FUNCTION_ENTRY;
  int ret = libMPI_Finalize();
  FUNCTION_EXIT;
  return ret;
}


/* internal function
 * This function is used by the various MPI_Init* functions (C
 * and Fortran versions)
 * This function add informations to the trace (rank, etc.)
 * and set the trace filename.
 */
void __mpi_init_generic() {
  int ret __attribute__((__unused__));
  libMPI_Comm_size(MPI_COMM_WORLD, &mpii_infos.size);
  libMPI_Comm_rank(MPI_COMM_WORLD, &mpii_infos.rank);
  mpii_infos.mpi_any_source = MPI_ANY_SOURCE;
  mpii_infos.mpi_any_tag = MPI_ANY_TAG;
  mpii_infos.mpi_proc_null = MPI_PROC_NULL;
  mpii_infos.mpi_request_null = MPI_REQUEST_NULL;
  mpii_infos.mpi_comm_world = MPI_COMM_WORLD;
  mpii_infos.mpi_comm_self = MPI_COMM_SELF;

  __mpi_init_called = 1;
}


int MPI_Init_thread(int* argc, char*** argv, int required, int* provided) {
  INTERCEPT_FUNCTION("MPI_Init_thread", libMPI_Init_thread);
  int ret = libMPI_Init_thread(argc, argv, required, provided);
  __mpi_init_generic();
  return ret;
}

int MPI_Init(int* argc, char*** argv) {
  INSTRUMENT_ALL_FUNCTIONS();
  int ret = libMPI_Init(argc, argv);
  __mpi_init_generic();
  return ret;
}
int MPI_Comm_disconnect(MPI_Comm* comm) {
  return libMPI_Comm_disconnect(comm);
}

int MPI_Comm_free(MPI_Comm* comm) {
  FUNCTION_ENTRY;
  int ret = libMPI_Comm_free(comm);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm* newcomm) {
  FUNCTION_ENTRY;
  int ret = libMPI_Comm_create(comm, group, newcomm);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Comm_create_group(MPI_Comm comm, MPI_Group group, int tag, MPI_Comm* newcomm) {
  FUNCTION_ENTRY;
  int ret = libMPI_Comm_create_group(comm, group, tag, newcomm);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm* newcomm) {
  FUNCTION_ENTRY;
  int ret = libMPI_Comm_split(comm, color, key, newcomm);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Comm_dup(MPI_Comm comm, MPI_Comm* newcomm) {
  FUNCTION_ENTRY;
  int ret = libMPI_Comm_dup(comm, newcomm);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Comm_dup_with_info(MPI_Comm comm, MPI_Info info, MPI_Comm* newcomm) {
  FUNCTION_ENTRY;
  int ret = libMPI_Comm_dup_with_info(comm, info, newcomm);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Comm_split_type(MPI_Comm comm, int split_type, int key, MPI_Info info,
                        MPI_Comm* newcomm) {
  FUNCTION_ENTRY;
  int ret = libMPI_Comm_split_type(comm, split_type, key, info, newcomm);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Intercomm_create(MPI_Comm local_comm, int local_leader,
                         MPI_Comm peer_comm, int remote_leader, int tag,
                         MPI_Comm* newintercomm) {
  FUNCTION_ENTRY;
  int ret = libMPI_Intercomm_create(local_comm, local_leader, peer_comm,
                                    remote_leader, tag, newintercomm);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Intercomm_merge(MPI_Comm intercomm, int high, MPI_Comm* newintracomm) {
  FUNCTION_ENTRY;
  int ret = libMPI_Intercomm_merge(intercomm, high, newintracomm);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Cart_sub(MPI_Comm old_comm, CONST int* belongs, MPI_Comm* new_comm) {
  FUNCTION_ENTRY;
  int ret = libMPI_Cart_sub(old_comm, belongs, new_comm);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Cart_create(MPI_Comm comm_old, int ndims, CONST int* dims,
                    CONST int* periods, int reorder, MPI_Comm* comm_cart) {
  FUNCTION_ENTRY;
  int ret = libMPI_Cart_create(comm_old, ndims, dims, periods, reorder,
                               comm_cart);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Graph_create(MPI_Comm comm_old, int nnodes, CONST int* index,
                     CONST int* edges, int reorder, MPI_Comm* comm_graph) {
  FUNCTION_ENTRY;
  int ret = libMPI_Graph_create(comm_old, nnodes, index, edges, reorder,
                                comm_graph);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Dist_graph_create(MPI_Comm comm_old,
                          int n,
                          CONST int sources[],
                          CONST int degrees[],
                          CONST int destinations[],
                          CONST int weights[],
                          MPI_Info info,
                          int reorder,
                          MPI_Comm* comm_dist_graph) {

  FUNCTION_ENTRY;
  int ret = libMPI_Dist_graph_create(comm_old, n, sources, degrees, destinations,
                                     weights, info, reorder, comm_dist_graph);
  FUNCTION_EXIT;
  return ret;
}

int MPI_Dist_graph_create_adjacent(MPI_Comm comm_old,
                                   int indegree,
                                   CONST int sources[],
                                   CONST int sourceweights[],
                                   int outdegree,
                                   CONST int destinations[],
                                   CONST int destweights[],
                                   MPI_Info info,
                                   int reorder,
                                   MPI_Comm* comm_dist_graph) {
  FUNCTION_ENTRY;
  int ret = libMPI_Dist_graph_create_adjacent(comm_old, indegree, sources,
                                              sourceweights, outdegree,
                                              destinations, destweights, info,
                                              reorder, comm_dist_graph);
  FUNCTION_EXIT;
  return ret;
}


PPTRACE_START_INTERCEPT_FUNCTIONS(mpi)
INTERCEPT3("MPI_Init_thread", libMPI_Init_thread)
INTERCEPT3("MPI_Init", libMPI_Init)
INTERCEPT3("MPI_Finalize", libMPI_Finalize)
INTERCEPT3("MPI_Barrier", libMPI_Barrier)
INTERCEPT3("MPI_Comm_size", libMPI_Comm_size)
INTERCEPT3("MPI_Comm_rank", libMPI_Comm_rank)
INTERCEPT3("MPI_Comm_get_parent", libMPI_Comm_get_parent)
INTERCEPT3("MPI_Type_size", libMPI_Type_size)

INTERCEPT3("MPI_Cancel", libMPI_Cancel)

INTERCEPT3("MPI_Comm_disconnect", libMPI_Comm_disconnect)
INTERCEPT3("MPI_Comm_free", libMPI_Comm_free)
INTERCEPT3("MPI_Comm_create", libMPI_Comm_create)
INTERCEPT3("MPI_Comm_create_group", libMPI_Comm_create_group)
INTERCEPT3("MPI_Comm_split", libMPI_Comm_split)
INTERCEPT3("MPI_Comm_dup", libMPI_Comm_dup)
INTERCEPT3("MPI_Comm_dup_with_info", libMPI_Comm_dup_with_info)
INTERCEPT3("MPI_Comm_split_type", libMPI_Comm_split_type)
INTERCEPT3("MPI_Intercomm_create", libMPI_Intercomm_create)
INTERCEPT3("MPI_Intercomm_merge", libMPI_Intercomm_merge)
INTERCEPT3("MPI_Cart_sub", libMPI_Cart_sub)
INTERCEPT3("MPI_Cart_create", libMPI_Cart_create)
INTERCEPT3("MPI_Graph_create", libMPI_Graph_create)
INTERCEPT3("MPI_Dist_graph_create", libMPI_Dist_graph_create)
INTERCEPT3("MPI_Dist_graph_create_adjacent", libMPI_Dist_graph_create_adjacent)

INTERCEPT3("MPI_Send", libMPI_Send)
INTERCEPT3("MPI_Recv", libMPI_Recv)

INTERCEPT3("MPI_Sendrecv", libMPI_Sendrecv)
INTERCEPT3("MPI_Sendrecv_replace", libMPI_Sendrecv_replace)

INTERCEPT3("MPI_Bsend", libMPI_Bsend)
INTERCEPT3("MPI_Ssend", libMPI_Ssend)
INTERCEPT3("MPI_Rsend", libMPI_Rsend)
INTERCEPT3("MPI_Isend", libMPI_Isend)
INTERCEPT3("MPI_Ibsend", libMPI_Ibsend)
INTERCEPT3("MPI_Issend", libMPI_Issend)
INTERCEPT3("MPI_Irsend", libMPI_Irsend)
INTERCEPT3("MPI_Irecv", libMPI_Irecv)

INTERCEPT3("MPI_Wait", libMPI_Wait)
INTERCEPT3("MPI_Waitall", libMPI_Waitall)
INTERCEPT3("MPI_Waitany", libMPI_Waitany)
INTERCEPT3("MPI_Waitsome", libMPI_Waitsome)
INTERCEPT3("MPI_Test", libMPI_Test)
INTERCEPT3("MPI_Testall", libMPI_Testall)
INTERCEPT3("MPI_Testany", libMPI_Testany)
INTERCEPT3("MPI_Testsome", libMPI_Testsome)

INTERCEPT3("MPI_Iprobe", libMPI_Iprobe)
INTERCEPT3("MPI_Probe", libMPI_Probe)

INTERCEPT3("MPI_Get", libMPI_Get)
INTERCEPT3("MPI_Put", libMPI_Put)

INTERCEPT3("MPI_Bcast", libMPI_Bcast)
INTERCEPT3("MPI_Gather", libMPI_Gather)
INTERCEPT3("MPI_Gatherv", libMPI_Gatherv)
INTERCEPT3("MPI_Scatter", libMPI_Scatter)
INTERCEPT3("MPI_Scatterv", libMPI_Scatterv)
INTERCEPT3("MPI_Allgather", libMPI_Allgather)
INTERCEPT3("MPI_Allgatherv", libMPI_Allgatherv)
INTERCEPT3("MPI_Alltoall", libMPI_Alltoall)
INTERCEPT3("MPI_Alltoallv", libMPI_Alltoallv)
INTERCEPT3("MPI_Reduce", libMPI_Reduce)
INTERCEPT3("MPI_Allreduce", libMPI_Allreduce)
INTERCEPT3("MPI_Reduce_scatter", libMPI_Reduce_scatter)
INTERCEPT3("MPI_Scan", libMPI_Scan)

#ifdef USE_MPI3
INTERCEPT3("MPI_Ibarrier", libMPI_Ibarrier)
INTERCEPT3("MPI_Ibcast", libMPI_Ibcast)
INTERCEPT3("MPI_Igather", libMPI_Igather)
INTERCEPT3("MPI_Igatherv", libMPI_Igatherv)
INTERCEPT3("MPI_Iscatter", libMPI_Iscatter)
INTERCEPT3("MPI_Iscatterv", libMPI_Iscatterv)
INTERCEPT3("MPI_Iallgather", libMPI_Iallgather)
INTERCEPT3("MPI_Iallgatherv", libMPI_Iallgatherv)
INTERCEPT3("MPI_Ialltoall", libMPI_Ialltoall)
INTERCEPT3("MPI_Ialltoallv", libMPI_Ialltoallv)
INTERCEPT3("MPI_Ireduce", libMPI_Ireduce)
INTERCEPT3("MPI_Iallreduce", libMPI_Iallreduce)
INTERCEPT3("MPI_Ireduce_scatter", libMPI_Ireduce_scatter)
INTERCEPT3("MPI_Iscan", libMPI_Iscan)
#endif

INTERCEPT3("MPI_Comm_spawn", libMPI_Comm_spawn)

INTERCEPT3("MPI_Send_init", libMPI_Send_init)
INTERCEPT3("MPI_Bsend_init", libMPI_Bsend_init)
INTERCEPT3("MPI_Ssend_init", libMPI_Ssend_init)
INTERCEPT3("MPI_Rsend_init", libMPI_Rsend_init)
INTERCEPT3("MPI_Recv_init", libMPI_Recv_init)
INTERCEPT3("MPI_Start", libMPI_Start)
INTERCEPT3("MPI_Startall", libMPI_Startall)

/* fortran binding */
INTERCEPT3("mpi_init_", libmpi_init_)
INTERCEPT3("mpi_init_thread_", libmpi_init_thread_)
INTERCEPT3("mpi_init_", libmpi_init_)
INTERCEPT3("mpi_finalize_", libmpi_finalize_)
INTERCEPT3("mpi_barrier_", libmpi_barrier_)
INTERCEPT3("mpi_comm_size_", libmpi_comm_size_)
INTERCEPT3("mpi_comm_rank_", libmpi_comm_rank_)
INTERCEPT3("mpi_comm_get_parent_", libmpi_comm_get_parent_)
INTERCEPT3("mpi_type_size_", libmpi_type_size_)

INTERCEPT3("mpi_cancel_", libmpi_cancel_)

INTERCEPT3("mpi_comm_create_", libmpi_comm_create_)
INTERCEPT3("mpi_comm_create_group_", libmpi_comm_create_group_)
INTERCEPT3("mpi_comm_split_", libmpi_comm_split_)
INTERCEPT3("mpi_comm_dup_", libmpi_comm_dup_)
INTERCEPT3("mpi_comm_dup_with_info_", libmpi_comm_dup_with_info_)
INTERCEPT3("mpi_comm_split_type_", libmpi_comm_split_type_)
INTERCEPT3("mpi_intercomm_create_", libmpi_intercomm_create_)
INTERCEPT3("mpi_intercomm_merge_", libmpi_intercomm_merge_)
INTERCEPT3("mpi_cart_sub_", libmpi_cart_sub_)
INTERCEPT3("mpi_cart_create_", libmpi_cart_create_)
INTERCEPT3("mpi_graph_create_", libmpi_graph_create_)
INTERCEPT3("mpi_dist_graph_create_", libmpi_dist_graph_create_)
INTERCEPT3("mpi_dist_graph_create_adjacent_", libmpi_dist_graph_create_adjacent_)

INTERCEPT3("mpi_send_", libmpi_send_)
INTERCEPT3("mpi_recv_", libmpi_recv_)

INTERCEPT3("mpi_sendrecv_", libmpi_sendrecv_)
INTERCEPT3("mpi_sendrecv_replace_", libmpi_sendrecv_replace_)
INTERCEPT3("mpi_bsend_", libmpi_bsend_)
INTERCEPT3("mpi_ssend_", libmpi_ssend_)
INTERCEPT3("mpi_rsend_", libmpi_rsend_)
INTERCEPT3("mpi_isend_", libmpi_isend_)
INTERCEPT3("mpi_ibsend_", libmpi_ibsend_)
INTERCEPT3("mpi_issend_", libmpi_issend_)
INTERCEPT3("mpi_irsend_", libmpi_irsend_)
INTERCEPT3("mpi_irecv_", libmpi_irecv_)

INTERCEPT3("mpi_wait_", libmpi_wait_)
INTERCEPT3("mpi_waitall_", libmpi_waitall_)
INTERCEPT3("mpi_waitany_", libmpi_waitany_)
INTERCEPT3("mpi_waitsome_", libmpi_waitsome_)
INTERCEPT3("mpi_test_", libmpi_test_)
INTERCEPT3("mpi_testall_", libmpi_testall_)
INTERCEPT3("mpi_testany_", libmpi_testany_)
INTERCEPT3("mpi_testsome_", libmpi_testsome_)

INTERCEPT3("mpi_probe_", libmpi_probe_)
INTERCEPT3("mpi_iprobe_", libmpi_iprobe_)

INTERCEPT3("mpi_get_", libmpi_get_)
INTERCEPT3("mpi_put_", libmpi_put_)

INTERCEPT3("mpi_bcast_", libmpi_bcast_)
INTERCEPT3("mpi_gather_", libmpi_gather_)
INTERCEPT3("mpi_gatherv_", libmpi_gatherv_)
INTERCEPT3("mpi_scatter_", libmpi_scatter_)
INTERCEPT3("mpi_scatterv_", libmpi_scatterv_)
INTERCEPT3("mpi_allgather_", libmpi_allgather_)
INTERCEPT3("mpi_allgatherv_", libmpi_allgatherv_)
INTERCEPT3("mpi_alltoall_", libmpi_alltoall_)
INTERCEPT3("mpi_alltoallv_", libmpi_alltoallv_)
INTERCEPT3("mpi_reduce_", libmpi_reduce_)
INTERCEPT3("mpi_allreduce_", libmpi_allreduce_)
INTERCEPT3("mpi_reduce_scatter_", libmpi_reduce_scatter_)
INTERCEPT3("mpi_scan_", libmpi_scan_)

#ifdef USE_MPI3
INTERCEPT3("mpi_ibarrier_", libmpi_ibarrier_)
INTERCEPT3("mpi_ibarrier_", libmpi_ibarrier_)
INTERCEPT3("mpi_ibcast_", libmpi_ibcast_)
INTERCEPT3("mpi_igather_", libmpi_igather_)
INTERCEPT3("mpi_igatherv_", libmpi_igatherv_)
INTERCEPT3("mpi_iscatter_", libmpi_iscatter_)
INTERCEPT3("mpi_iscatterv_", libmpi_iscatterv_)
INTERCEPT3("mpi_iallgather_", libmpi_iallgather_)
INTERCEPT3("mpi_iallgatherv_", libmpi_iallgatherv_)
INTERCEPT3("mpi_ialltoall_", libmpi_ialltoall_)
INTERCEPT3("mpi_ialltoallv_", libmpi_ialltoallv_)
INTERCEPT3("mpi_ireduce_", libmpi_ireduce_)
INTERCEPT3("mpi_iallreduce_", libmpi_iallreduce_)
INTERCEPT3("mpi_ireduce_scatter_", libmpi_ireduce_scatter_)
INTERCEPT3("mpi_iscan_", libmpi_iscan_)
#endif

INTERCEPT3("mpi_comm_spawn_", libmpi_comm_spawn_)

INTERCEPT3("mpi_send_init_", libmpi_send_init_)
INTERCEPT3("mpi_bsend_init_", libmpi_bsend_init_)
INTERCEPT3("mpi_ssend_init_", libmpi_ssend_init_)
INTERCEPT3("mpi_rsend_init_", libmpi_rsend_init_)
INTERCEPT3("mpi_recv_init_", libmpi_recv_init_)
INTERCEPT3("mpi_start_", libmpi_start_)
INTERCEPT3("mpi_startall_", libmpi_startall_)
PPTRACE_END_INTERCEPT_FUNCTIONS(mpi)

extern char**environ;
char ld_preload_value[4096];

/* unset LD_PRELOAD
 * this makes sure that forked processes will not be analyzed
 */
static void unset_ld_preload() {
  /* unset LD_PRELOAD */
  char* ld_preload = getenv("LD_PRELOAD");
  if(!ld_preload) {
    ld_preload_value[0]='\0';
    return;
  }

  /* save the value of ld_preload so that we can set it back later */
  strncpy(ld_preload_value, ld_preload, 4096);
  int ret = unsetenv("LD_PRELOAD");
  if(ret != 0 ){
    fprintf(stderr, "unsetenv failed ! %s\n", strerror(errno));
    abort();
  }

  /* also change the environ variable since exec* function
   * rely on it.
   */
  for (int i=0; environ[i]; i++) {
    if (strstr(environ[i],"LD_PRELOAD=")) {
      printf("hacking out LD_PRELOAD from environ[%d]\n",i);
      environ[i][0] = '\0';
    }
  }
  char*plop=getenv("LD_PRELOAD");
  if(plop) {
    fprintf(stderr, "Warning: cannot unset LD_PRELOAD\n");
    fprintf(stderr, "This is likely to cause problems later.\n");
  }
}

/* set LD_PRELOAD so that future forked processes are analyzed
 *  you need to call unset_ld_preload before calling this function
 */
static void reset_ld_preload() {
  if(strlen(ld_preload_value)>0) {
    MPII_PRINTF(1, "Setting back ld_preload to %s\n", ld_preload_value);
    setenv("LD_PRELOAD", ld_preload_value, 1);
  }
}

static void load_settings() {
  char* mpii_verbose = getenv("MPII_VERBOSE");
  if(mpii_verbose) {
    mpii_infos.settings.verbose = atoi(mpii_verbose);
    printf("[MPII] Debug level: %d\n", mpii_infos.settings.verbose);
  }
}

void mpii_init(void) __attribute__((constructor));
void mpii_init(void) {
  mpii_infos.settings.verbose=SETTINGS_VERBOSE_DEFAULT;
  unset_ld_preload();
  load_settings();  
  INSTRUMENT_ALL_FUNCTIONS();
}

void mpii_finish(void) __attribute__((destructor));
void mpii_finish(void) {
}

