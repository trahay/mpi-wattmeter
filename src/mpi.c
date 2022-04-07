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
#include <sys/syscall.h>
#include <linux/perf_event.h>

static int perf_event_open(struct perf_event_attr *hw_event_uptr,
                    pid_t pid, int cpu, int group_fd, unsigned long flags) {

        return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
                        group_fd, flags);
}
#define MAX_CPUS	1024
#define MAX_PACKAGES	16
#define NUM_RAPL_DOMAINS	5
static int total_cores=0,total_packages=0;
static int package_map[MAX_PACKAGES];

static int detect_packages(void) {
  char filename[BUFSIZ];
  FILE *fff;
  int package;
  int i;

  for(i=0;i<MAX_PACKAGES;i++) package_map[i]=-1;

  printf("\t");
  for(i=0;i<MAX_CPUS;i++) {
    sprintf(filename,"/sys/devices/system/cpu/cpu%d/topology/physical_package_id",i);
    fff=fopen(filename,"r");
    if (fff==NULL) break;
    fscanf(fff,"%d",&package);
    printf("%d (%d)",i,package);
    if (i%8==7) printf("\n\t"); else printf(", ");
    fclose(fff);

    if (package_map[package]==-1) {
      total_packages++;
      package_map[package]=i;
    }

  }

  printf("\n");

  total_cores=i;

  printf("\tDetected %d cores in %d packages\n\n",
	 total_cores,total_packages);

  return 0;
}

char rapl_domain_names[NUM_RAPL_DOMAINS][30]= {
	"energy-cores",
	"energy-gpu",
	"energy-pkg",
	"energy-ram",
	"energy-psys",
};

static int check_paranoid(void) {
  int paranoid_value;
  FILE *fff;

  fff=fopen("/proc/sys/kernel/perf_event_paranoid","r");
  if (fff==NULL) {
    fprintf(stderr,"Error! could not open /proc/sys/kernel/perf_event_paranoid %s\n",
	    strerror(errno));

    /* We can't return a negative value as that implies no paranoia */
    return 500;
  }

  fscanf(fff,"%d",&paranoid_value);
  fclose(fff);

  return paranoid_value;
}

double scale[NUM_RAPL_DOMAINS];
int fd[NUM_RAPL_DOMAINS][MAX_PACKAGES];
char units[NUM_RAPL_DOMAINS][BUFSIZ];

static int start_rapl_perf() {
  FILE *fff;
  int type;
  int config[NUM_RAPL_DOMAINS];
  char filename[BUFSIZ];
  struct perf_event_attr attr;
  long long value;
  int i,j;
  int paranoid_value;

  fff=fopen("/sys/bus/event_source/devices/power/type","r");
  if (fff==NULL) {
    printf("\tNo perf_event rapl support found (requires Linux 3.14)\n");
    printf("\tFalling back to raw msr support\n\n");
    return -1;
  }
  fscanf(fff,"%d",&type);
  fclose(fff);

  for(i=0;i<NUM_RAPL_DOMAINS;i++) {

    sprintf(filename,"/sys/bus/event_source/devices/power/events/%s",
	    rapl_domain_names[i]);

    fff=fopen(filename,"r");

    if (fff!=NULL) {
      fscanf(fff,"event=%x",&config[i]);
      //			printf("\tEvent=%s Config=%d ",rapl_domain_names[i],config[i]);
      fclose(fff);
    } else {
      continue;
    }

    sprintf(filename,"/sys/bus/event_source/devices/power/events/%s.scale",
	    rapl_domain_names[i]);
    fff=fopen(filename,"r");

    if (fff!=NULL) {
      fscanf(fff,"%lf",&scale[i]);
      printf("scale=%g ",scale[i]);
      fclose(fff);
    }

    sprintf(filename,"/sys/bus/event_source/devices/power/events/%s.unit",
	    rapl_domain_names[i]);
    fff=fopen(filename,"r");

    if (fff!=NULL) {
      fscanf(fff,"%s",units[i]);
      printf("units=%s ",units[i]);
      fclose(fff);
    }

    printf("\n");
  }

  for(j=0;j<total_packages;j++) {

    for(i=0;i<NUM_RAPL_DOMAINS;i++) {

      fd[i][j]=-1;

      memset(&attr,0x0,sizeof(attr));
      attr.type=type;
      attr.config=config[i];
      if (config[i]==0) continue;

      fd[i][j]=perf_event_open(&attr,-1, package_map[j],-1,0);
      if (fd[i][j]<0) {
	if (errno==EACCES) {
	  paranoid_value=check_paranoid();
	  if (paranoid_value>0) {
	    printf("\t/proc/sys/kernel/perf_event_paranoid is %d\n",paranoid_value);
	    printf("\tThe value must be 0 or lower to read system-wide RAPL values\n");
	  }

	  printf("\tPermission denied; run as root or adjust paranoid value\n\n");
	  return -1;
	}
	else {
	  printf("\terror opening core %d config %d: %s\n\n",
		 package_map[j], config[i], strerror(errno));
	  return -1;
	}
      }
    }
  }

  return 0;
}

double joules_to_watthour(double joules) {
  return joules/3600;
}

static int stop_rapl_perf() {
  long long value;
  for(int j=0;j<total_packages;j++) {
    printf("\tPackage %d:\n",j);

    for(int i=0;i<NUM_RAPL_DOMAINS;i++) {

      if (fd[i][j]!=-1) {
	read(fd[i][j],&value,8);
	close(fd[i][j]);

	printf("\t\t%s Energy Consumed: %lf %s (%lf watts.hour)\n",
	       rapl_domain_names[i],
	       (double)value*scale[i],
	       units[i],
	       joules_to_watthour((double)value*scale[i]));

      }

    }

  }
  printf("\n");

}
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

int MPI_Finalize() {
  FUNCTION_ENTRY;
  printf("[%d/%d] MPI_Init\n", mpii_infos.rank, mpii_infos.size);
  if(mpii_infos.rank == 0)
    stop_rapl_perf();

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
  __mpi_init_called = 1;
  MPI_Comm_size(MPI_COMM_WORLD, &mpii_infos.size);
  MPI_Comm_rank(MPI_COMM_WORLD, &mpii_infos.rank);
  printf("[%d/%d] MPI_Init\n", mpii_infos.rank, mpii_infos.size);

  if(mpii_infos.rank == 0) {
    detect_packages(); 
    start_rapl_perf();
  }
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


PPTRACE_START_INTERCEPT_FUNCTIONS(mpi)
INTERCEPT3("MPI_Init_thread", libMPI_Init_thread)
INTERCEPT3("MPI_Init", libMPI_Init)
INTERCEPT3("MPI_Finalize", libMPI_Finalize)
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

