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

struct ezt_instrumented_function {
  char function_name[1024];
  void* callback;
  int event_id;
};

extern struct ezt_instrumented_function hijack_list[];
#define INSTRUMENTED_FUNCTIONS hijack_list

struct mpii_info {
  int rank;
  int size;

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

/* maximum number of items to be allocated statically
 * if the application need more than this, a dynamic array
 * is allocated using malloc()
 */
#define MAX_REQS 128

/* allocate a number of elements using a static array if possible
 * if not possible (ie. count>MAX_REQS) use a dynamic array (ie. malloc)
 */
#define ALLOCATE_ITEMS(type, count, static_var, dyn_var)	\
  type static_var[MAX_REQS];					\
  type* dyn_var = static_var;					\
  if ((count) > MAX_REQS)					\
    dyn_var = (type*)malloc(sizeof(type) * (count))

/* Free an array created by ALLOCATE_ITEMS */
#define FREE_ITEMS(count, dyn_var)		\
  if ((count) > MAX_REQS)			\
    free(dyn_var)

/* pointers to actual MPI functions (C version)  */
extern int (*libMPI_Init)(int*, char***);
extern int (*libMPI_Init_thread)(int*, char***, int, int*);
extern int (*libMPI_Finalize)(void);

/* return 1 if buf corresponds to the Fotran MPI_IN_PLACE
 * return 0 otherwise
 */
int ezt_mpi_is_in_place_(void* buf);

/* check the value of a Fortran pointer and return MPI_IN_PLACE or p
 */
#define CHECK_MPI_IN_PLACE(p) (ezt_mpi_is_in_place_(p) ? MPI_IN_PLACE : (p))


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


