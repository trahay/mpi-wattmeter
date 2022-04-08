#pragma once
#define _GNU_SOURCE
#include <dlfcn.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <mpi.h>
#include "mpii_macros.h"
#include "mpii_config.h"


enum rapl_domain {
  energy_cores,
  energy_gpu,
  energy_pkg,
  energy_ram,
  energy_psys,
  NUM_RAPL_DOMAINS
};

static char rapl_domain_names[NUM_RAPL_DOMAINS][30] MAYBE_UNUSED = {
  "energy-cores",
  "energy-gpu",
  "energy-pkg",
  "energy-ram",
  "energy-psys",
};

struct rapl_measurement {
  /* energy consummed by the domain (in Joule) */
  double counter_value[NUM_RAPL_DOMAINS];
  //  char hostname[MPI_MAX_PROCESSOR_NAME];
};

struct nvidia_measurement {
  char device_name[32];
  double energy;	/* energy consumed by the GPU (in joule) */
};

struct mpii_info {
  int rank;
  int size;
  char hostname[MPI_MAX_PROCESSOR_NAME];
  char *hostnames;

  int nb_gpus;
  struct nvidia_measurement* gpu_measurement;
  struct rapl_measurement rapl_measurement;

  int mpi_mode;	/* are we running an MPI application ? */
  int is_local_master;		/* set to 1 if the rank is a local master */
  MPI_Comm local_master_comm;	/* communicator for the local masters */
  int local_rank;		/* rank in the communicator */
  int local_size;		/* size of the communicator */

  struct mpii_settings settings;
};

/* information on the local process */
extern struct mpii_info mpii_infos;

#if 1
#define FUNCTION_ENTRY_(fname) MPII_PRINTF(2, "[%d/%d]\tEntering %s\n", mpii_infos.rank, mpii_infos.size, fname);
#define FUNCTION_EXIT_(fname)  MPII_PRINTF(2, "[%d/%d]\tLeaving %s\n", mpii_infos.rank, mpii_infos.size, fname);
#else
#define FUNCTION_ENTRY_(fname) (void)0
#define FUNCTION_EXIT_(fname)  (void)0
#endif

#define MPII_PRINTF(_debug_level_, ...)            \
  {                                               \
    if (mpii_infos.settings.verbose >= _debug_level_) \
      fprintf(stderr, __VA_ARGS__);               \
  }

#define FUNCTION_ENTRY FUNCTION_ENTRY_(__func__);
#define FUNCTION_EXIT  FUNCTION_EXIT_(__func__);

/* pointers to actual MPI functions (C version)  */
extern int (*libMPI_Init)(int*, char***);
extern int (*libMPI_Init_thread)(int*, char***, int, int*);
extern int (*libMPI_Finalize)(void);


struct ezt_instrumented_function {
  char function_name[1024];
  void* callback;
  int event_id;
};

extern struct ezt_instrumented_function hijack_list[];
#define INSTRUMENTED_FUNCTIONS hijack_list


#define PPTRACE_START_INTERCEPT_FUNCTIONS(module_name) struct ezt_instrumented_function INSTRUMENTED_FUNCTIONS [] = {

  
#define PPTRACE_END_INTERCEPT_FUNCTIONS(module_name) \
  FUNCTION_NONE					     \
  }                                        \
  ;

#define INTERCEPT3(func, var) {	\
    .function_name=func,			\
      .callback=&(var),				\
      .event_id = -1,				\
      },

#define FUNCTION_NONE  {			\
    .function_name="",				\
      .callback=NULL,				\
      .event_id = -1,				\
      },

static void instrument_function(struct ezt_instrumented_function* f) __attribute__((unused));
static void instrument_functions(struct ezt_instrumented_function* functions) __attribute__((unused));
static struct ezt_instrumented_function* find_instrumented_function(const char* fname, struct ezt_instrumented_function* functions) __attribute__((unused));

static void instrument_function(struct ezt_instrumented_function* f) {
  
  if(f->event_id >= 0) {
    /* this function has already been initialized */
    return;
  }

  assert(f->callback != NULL);

  static __thread int recursion_shield = 0;
  recursion_shield++;
  if(recursion_shield == 1) {
    if(*(void**)f->callback == NULL) {
      MPII_PRINTF(1, "Instrumenting %s using dlsym\n", f->function_name);
      /* binary instrumentation did not find the symbol. */
      void* ptr = dlsym(RTLD_NEXT, f->function_name);
      if(ptr) {
	memcpy(f->callback, &ptr, sizeof(void*));
      }
    }
  }
  recursion_shield--;
}

static struct ezt_instrumented_function* find_instrumented_function(const char* fname, struct ezt_instrumented_function* functions) {
  struct ezt_instrumented_function*f=NULL;
  for(f = functions;
      strcmp(f->function_name, "") != 0;
      f++) {
    if(strcmp(f->function_name, fname) == 0) {
      return f;
    }
  }
  return NULL;
}

static void instrument_functions(struct ezt_instrumented_function* functions) {
  struct ezt_instrumented_function*f=NULL;

  for(f = functions;
      strcmp(f->function_name, "") != 0;
      f++) {
    instrument_function(f);
  }
}

#define INSTRUMENT_ALL_FUNCTIONS() do {		\
  instrument_functions(INSTRUMENTED_FUNCTIONS);	\
  }while(0);


/* instrument one function */
#define INTERCEPT_FUNCTION(fname, cb) do {				\
    if(!cb) {								\
      struct ezt_instrumented_function*f=NULL;				\
      for(f = INSTRUMENTED_FUNCTIONS;					\
	  strcmp(f->function_name, fname) != 0 &&			\
	    strcmp(f->function_name, "") != 0;				\
	  f++) {							\
      }									\
      instrument_function(f);						\
    }									\
} while(0)


/* maximum number of joules. If more than than , there's probably a bug */
#define MAX_VALUE 1e30

static inline double joules_to_watthour(double joules) {
  return joules/3600;
}


void start_measurements();
void stop_measurements();
void print_measurements();
 
/* Initialize RAPL */
int mpi_rapl_init();
/* Start the RAPL measurement */
int mpi_rapl_start();
/* Stop the RAPL measurement */
int mpi_rapl_stop(struct rapl_measurement *m);

/* initialize NVML.
 * Return the number of GPUs
 */
int mpi_nvml_init();

/* Start NVML measurement */
int mpi_nvml_start(struct nvidia_measurement* m);

/* Stop NVML measurement */
int mpi_nvml_stop(struct nvidia_measurement* m);

static inline char* get_rank_hostname(int rank) {
  return &mpii_infos.hostnames[rank * MPI_MAX_PROCESSOR_NAME];
}
