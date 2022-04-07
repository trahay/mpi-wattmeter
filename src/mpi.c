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
int (*libMPI_Finalize)(void);

/* fortran bindings */
void (*libmpi_init_)(int* e);
void (*libmpi_init_thread_)(int*, int*, int*);
void (*libmpi_finalize_)(int*);

static int __mpi_init_called = 0;


/* internal function
 * This function is used by the various MPI_Init* functions (C
 * and Fortran versions)
 * This function add informations to the trace (rank, etc.)
 * and set the trace filename.
 */
void __mpi_init_generic() {
  if(__mpi_init_called) return;

  __mpi_init_called = 1;
  MPI_Comm_size(MPI_COMM_WORLD, &mpii_infos.size);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpii_infos.rank);
  printf("[%d/%d] MPI_Init\n", mpii_infos.rank, mpii_infos.size);
  
  if(mpii_infos.rank == 0) {
    start_rapl_perf();
  }
}


int MPI_Finalize() {
  FUNCTION_ENTRY;
  printf("[%d/%d] MPI_Init\n", mpii_infos.rank, mpii_infos.size);
  if(mpii_infos.rank == 0) {
    struct rapl_measurement m;
    stop_rapl_perf(&m);
    print_rapl_measurement(&m);
  }

  int ret = libMPI_Finalize();
  FUNCTION_EXIT;
  return ret;
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

/* called by Fortran verison of MPI_Init */
void mpif_init_(void* error) {
  INSTRUMENT_ALL_FUNCTIONS();
  libmpi_init_((int*)error);
  __mpi_init_generic();
}

/* called by Fortran verison of MPI_Init_thread */
void mpif_init_thread_(int* r, int* p, int* error) {
  INSTRUMENT_ALL_FUNCTIONS();
  libmpi_init_thread_(r, p, error);
  __mpi_init_generic();
}

/* called by Fortran verison of MPI_Finalize */
void mpif_finalize_(int* error) {
  *error = MPI_Finalize();
}

PPTRACE_START_INTERCEPT_FUNCTIONS(mpi)
INTERCEPT3("MPI_Init_thread", libMPI_Init_thread)
INTERCEPT3("MPI_Init", libMPI_Init)
INTERCEPT3("MPI_Finalize", libMPI_Finalize)
/* fortran binding */
INTERCEPT3("mpi_init_", libmpi_init_)
INTERCEPT3("mpi_init_thread_", libmpi_init_thread_)
INTERCEPT3("mpi_finalize_", libmpi_finalize_)
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

