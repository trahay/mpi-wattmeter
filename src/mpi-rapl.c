#include "mpii.h"

#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>


#define MAX_CPUS	1024
#define MAX_PACKAGES	16


static int total_cores=0; // number of cores
static int total_packages=0; // number of packages (eg. sockets)
static int package_map[MAX_PACKAGES];


double scale[NUM_RAPL_DOMAINS];
int fd[NUM_RAPL_DOMAINS][MAX_PACKAGES];
char units[NUM_RAPL_DOMAINS][BUFSIZ];

static int perf_event_open(struct perf_event_attr *hw_event_uptr,
                    pid_t pid, int cpu, int group_fd, unsigned long flags) {

  return syscall(__NR_perf_event_open,hw_event_uptr, pid, cpu,
		 group_fd, flags);
}

static int detect_packages(void) {
  static int initialized = 0;
  if(initialized) return 0;

  initialized++;

  char filename[BUFSIZ];
  FILE *fff;
  int package;
  int i;

  for(i=0;i<MAX_PACKAGES;i++)
    package_map[i]=-1;

  for(i=0;i<MAX_CPUS;i++) {
    sprintf(filename,"/sys/devices/system/cpu/cpu%d/topology/physical_package_id",i);
    fff=fopen(filename,"r");
    if (fff==NULL) break;

    fscanf(fff,"%d",&package);
    fclose(fff);

    if (package_map[package]==-1) {
      total_packages++;
      package_map[package]=i;
    }

  }
  total_cores=i;

//  printf("[%d/%d] \tDetected %d cores in %d packages\n\n",
//	 mpii_infos.rank, mpii_infos.size, total_cores,total_packages);

  return 0;
}


int check_paranoid(void) {
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

int start_rapl_perf() {
  if( detect_packages() < 0 ) {
    printf("detect packge failed\n");
    return -1;
  }
  if( check_paranoid() == 500) {
    printf("Check paranoid failed\n");
    return -1;
  }

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
      fclose(fff);
    } else {
      continue;
    }

    sprintf(filename,"/sys/bus/event_source/devices/power/events/%s.scale",
	    rapl_domain_names[i]);
    fff=fopen(filename,"r");

    if (fff!=NULL) {
      fscanf(fff,"%lf",&scale[i]);
      fclose(fff);
    }

    sprintf(filename,"/sys/bus/event_source/devices/power/events/%s.unit",
	    rapl_domain_names[i]);
    fff=fopen(filename,"r");

    if (fff!=NULL) {
      fscanf(fff,"%s",units[i]);
      fclose(fff);
    }
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

#define MAX_VALUE 1e30

int stop_rapl_perf(struct rapl_measurement *m) {
  long long value;
  int  namelen;
  MPI_Get_processor_name(m->hostname,&namelen);

  for(int j=0;j<total_packages;j++) {
    for(int i=0;i<NUM_RAPL_DOMAINS;i++) {
      if (fd[i][j]!=-1) {
	read(fd[i][j],&value,8);
	close(fd[i][j]);
	m->counter_value[i] = (double)value*scale[i];

	if(value > MAX_VALUE) {
	  printf("Wow, that's a lot of joules ! (%"PRIu64")\n", value);
	  abort();
	}

      }
    }
  }
  return 0;
}

void print_rapl_measurement(struct rapl_measurement *m, int mpi_rank) {
  long long value;
  for(int i=0;i<NUM_RAPL_DOMAINS;i++) {
    if(m->counter_value[i] > 0 ) {
      printf("[%s#%d]\t\t%s Energy Consumed: %lf %s (%lf watts.hour)\n",
	     m->hostname, mpi_rank,
	     rapl_domain_names[i],
	     m->counter_value[i],
	     units[i],
	     joules_to_watthour(m->counter_value[i]));
      if(m->counter_value[i] > MAX_VALUE) {
	printf("Wow, that's a lot of joules ! (%"PRIu64")\n", (uint64_t)m->counter_value[i]);
	abort();
      }
    }
  }
  printf("\n");
}

static void init_measurement(struct rapl_measurement *m) {
  for(int i=0;i<NUM_RAPL_DOMAINS;i++) {
    m->counter_value[i] = 0;
  }
  m->hostname[0]='\0';
}

void print_rapl_measurements(struct rapl_measurement *m, int nb) {
  struct rapl_measurement min;
  struct rapl_measurement max;
  struct rapl_measurement total;
  struct rapl_measurement avg;
  init_measurement(&min);
  init_measurement(&max);
  init_measurement(&total);
  init_measurement(&avg);

  for(int j=0;j<NUM_RAPL_DOMAINS;j++) {
    int nb_val = 0;
    for(int i=0; i<nb; i++) {
      if(m[i].counter_value[j] > 0 ) {
	nb_val++;
	if(m[i].counter_value[j] < min.counter_value[j] || min.counter_value[j] == 0)
	  min.counter_value[j] = m[i].counter_value[j];

	if(m[i].counter_value[j] > max.counter_value[j] || max.counter_value[j] == 0)
	  max.counter_value[j] = m[i].counter_value[j];

	total.counter_value[j] += m[i].counter_value[j];
      }
    }
    if(nb_val > 0 ) {
      avg.counter_value[j]=total.counter_value[j]/nb_val;
      printf("[Total]\t\t%s Energy Consumed (total/avg/min/max): %lf/%lf/%lf/%lf %s (%lf/%lf/%lf/%lf watts.hour)\n",
	     rapl_domain_names[j],
	     total.counter_value[j],
	     avg.counter_value[j],
	     min.counter_value[j],
	     max.counter_value[j],
	     units[j],
	     joules_to_watthour(total.counter_value[j]),
	     joules_to_watthour(avg.counter_value[j]),
	     joules_to_watthour(min.counter_value[j]),
	     joules_to_watthour(max.counter_value[j])
	     );
    }
  }
}
