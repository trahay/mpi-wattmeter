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
#include <inttypes.h>

#include <mpi.h>

/* pointers to actual MPI functions (C version)  */
int (*libMPI_Init)(int*, char***);
int (*libMPI_Init_thread)(int*, char***, int, int*);
int (*libMPI_Finalize)(void);

/* fortran bindings */
void (*libmpi_init_)(int* e);
void (*libmpi_init_thread_)(int*, int*, int*);
void (*libmpi_finalize_)(int*);

static int __mpi_init_called = 0;

/* Create a new communicator that contains one MPI rank (called local-master) 
 * per compute node.
 *
 * return 1 if the current rank is a local-master or 0 otherwise
 */
static int create_communicator() {
  int rank = mpii_infos.rank;
  int nprocs= mpii_infos.size;

  /* Create an array that contains all the hostnames */ 
  size_t bytes = nprocs * sizeof(char[MPI_MAX_PROCESSOR_NAME]);

  mpii_infos.hostnames = malloc(bytes);
  strcpy(get_rank_hostname(rank), mpii_infos.hostname);
 
  for (int n=0; n<nprocs; n++) { /* TODO: replace with a call to alltoall */
    MPI_Bcast(get_rank_hostname(n),MPI_MAX_PROCESSOR_NAME, MPI_CHAR, n, MPI_COMM_WORLD);
  }


  /* color=0 means that we do not belong to the new communicator
   */
  int color = 0;
  int found = 0;
  /* Browse the hostnames array. Until the local hostname is found.
   */
  for (int n=0; n<nprocs && (!found) ; n++) {
    if(strcmp(get_rank_hostname(n), mpii_infos.hostname) == 0) {
      if(n==rank) {
	/* i'm the local master */
	color = 1;
      }
      found = 1;
    }
  }

  /* Create a communicator with all the "local master" ranks */
  MPI_Comm_split(MPI_COMM_WORLD,
		 color,
		 rank,
		 &mpii_infos.local_master_comm);
  if(color) {
    MPI_Comm_rank(mpii_infos.local_master_comm, &mpii_infos.local_rank);
    MPI_Comm_size(mpii_infos.local_master_comm, &mpii_infos.local_size);
    mpii_infos.is_local_master = 1;
    return 1;
  } else {
    /* non local-master ranks don't need the communicator */
    mpii_infos.local_size = -1;
    mpii_infos.local_rank = -1;
    mpii_infos.is_local_master = 0;
    MPI_Comm_free(&mpii_infos.local_master_comm);
    return 0;
  }
}


/* internal function
 * This function is used by the various MPI_Init* functions (C
 * and Fortran versions)
 * This function add informations to the trace (rank, etc.)
 * and set the trace filename.
 */
void __mpi_init_generic() {
  if(__mpi_init_called) return;

  __mpi_init_called = 1;
  mpii_infos.mpi_mode = 1;
  MPI_Comm_size(MPI_COMM_WORLD, &mpii_infos.size);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpii_infos.rank);

  create_communicator(&mpii_infos.local_master_comm);
  if(mpii_infos.is_local_master) {
    start_measurements();
  }
}

int MPI_Finalize() {
  FUNCTION_ENTRY;
  stop_measurements();
  print_measurements();

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
static void reset_ld_preload() MAYBE_UNUSED;
static void reset_ld_preload() {  
  if(strlen(ld_preload_value)>0) {
    MPII_PRINTF(1, "Setting back ld_preload to %s\n", ld_preload_value);
    setenv("LD_PRELOAD", ld_preload_value, 1);
  }
}

static void load_settings() {

#define GET_INT_OPTION_VALUE(variable, option, default_value) do {	\
    variable = default_value;						\
    char* str = getenv(option);						\
    if(str) {								\
      variable  = atoi(str);						\
    }								\
  } while(0)

#define GET_STRING_OPTION_VALUE(variable, buffer_size,  option, default_value) do { \
    strncpy(variable, default_value, buffer_size);			\
    char* str = getenv(option);						\
    if(str) {								\
      strncpy(variable, str, buffer_size);				\
    }									\
  } while(0)

  GET_INT_OPTION_VALUE(mpii_infos.settings.verbose, "MPII_VERBOSE", SETTINGS_VERBOSE_DEFAULT);
  GET_INT_OPTION_VALUE(mpii_infos.settings.print_details, "MPII_PRINT_DETAILS", SETTINGS_PRINT_DETAILS_DEFAULT);
  GET_INT_OPTION_VALUE(mpii_infos.settings.print_joules, "MPII_PRINT_JOULES", SETTINGS_PRINT_JOULES_DEFAULT);
  GET_INT_OPTION_VALUE(mpii_infos.settings.print_watthours, "MPII_PRINT_WATTHOURS", SETTINGS_PRINT_WATTHOURS_DEFAULT);
  GET_INT_OPTION_VALUE(mpii_infos.settings.print_co2, "MPII_PRINT_CO2", SETTINGS_PRINT_CO2_DEFAULT);
  GET_INT_OPTION_VALUE(mpii_infos.settings.print_watt, "MPII_PRINT_WATT", SETTINGS_PRINT_WATT_DEFAULT);
  GET_STRING_OPTION_VALUE(mpii_infos.settings.plugin_list, STRING_LENGTH, "MPII_PLUGIN_LIST", SETTINGS_PLUGIN_LIST_DEFAULT);
}


void mpii_init(void) __attribute__((constructor));
void mpii_init(void) {
  unset_ld_preload();
  load_settings();  
  INSTRUMENT_ALL_FUNCTIONS();

  size_t namelen = MPI_MAX_PROCESSOR_NAME;
  gethostname(mpii_infos.hostname, namelen);

  load_plugins(&mpii_infos);
  start_measurements();
}

void mpii_finish(void) __attribute__((destructor));
void mpii_finish(void) {
  stop_measurements();
  print_measurements();
}

