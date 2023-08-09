#include "mpii.h"

#include <sys/syscall.h>
#include <linux/perf_event.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_CPUS	1024
#define MAX_PACKAGES	16


static int total_cores=0; // number of cores
static int total_packages=0; // number of packages (eg. sockets)
static int package_map[MAX_PACKAGES];


double scale[NUM_RAPL_DOMAINS];
int fd[NUM_RAPL_DOMAINS][MAX_PACKAGES];
char units[NUM_RAPL_DOMAINS][BUFSIZ];
static struct timespec start_date;

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

#if 0

int mpi_rapl_init(struct mpii_info *mpii_info) {
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
  unsigned int config[NUM_RAPL_DOMAINS];
  char filename[BUFSIZ];
  struct perf_event_attr attr;
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

  int domain_available[NUM_RAPL_DOMAINS];
  for(i=0;i<NUM_RAPL_DOMAINS;i++) {
    domain_available[i] = 0;
    sprintf(filename,"/sys/bus/event_source/devices/power/events/%s",
	    rapl_domain_names[i]);

    fff=fopen(filename,"r");

    if (fff!=NULL) {
      printf("[%d/%d] %s is available\n", i, NUM_RAPL_DOMAINS, rapl_domain_names[i]);
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
    domain_available[i] = 1;
  }

  for(j=0;j<total_packages;j++) {

    for(i=0;i<NUM_RAPL_DOMAINS;i++) {

      fd[i][j]=-1;

      if(domain_available[i]) {
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
	    printf("i=%d (%s)\n", i, rapl_domain_names[i]);
	    printf("\terror opening core %d (%s) config %d: %s\n\n",
		   package_map[j], rapl_domain_names[i], config[i], strerror(errno));
	    return -1;
	  }
	}
      }
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &start_date);

 return 0;
}

int mpi_rapl_start() {
  return 0;
}

int mpi_rapl_stop(struct rapl_measurement *m) {
  long long value;

  struct timespec stop_date;
  clock_gettime(CLOCK_MONOTONIC, &stop_date);
  m->period = (stop_date.tv_sec-start_date.tv_sec)+((stop_date.tv_nsec-start_date.tv_nsec)/1e9);
  for(int i=0;i<NUM_RAPL_DOMAINS;i++) {
    m->counter_value[i] = 0;
    for(int j=0;j<total_packages;j++) {
      if (fd[i][j]!=-1) {
	read(fd[i][j],&value,8);
	close(fd[i][j]);
	m->counter_value[i] += (double)value*scale[i];

	if(value > MAX_VALUE) {
	  printf("Wow, that's a lot of joules ! (%lld)\n", value);
	  abort();
	}
      }
    }
  }

  return 0;
}
#endif

struct measurement_plugin rapl_plugin;

int mpi_rapl_init(struct mpii_info *mpii_info) {
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
  unsigned int config[NUM_RAPL_DOMAINS];
  char filename[BUFSIZ];
  struct perf_event_attr attr;
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

  int domain_available[NUM_RAPL_DOMAINS];
  for(i=0;i<NUM_RAPL_DOMAINS;i++) {
    domain_available[i] = 0;
    sprintf(filename,"/sys/bus/event_source/devices/power/events/%s",
	    rapl_domain_names[i]);

    fff=fopen(filename,"r");

    if (fff!=NULL) {
      printf("[%d/%d] %s is available\n", i, NUM_RAPL_DOMAINS, rapl_domain_names[i]);
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
    domain_available[i] = 1;
  }

  for(j=0;j<total_packages;j++) {

    for(i=0;i<NUM_RAPL_DOMAINS;i++) {

      fd[i][j]=-1;

      if(domain_available[i]) {
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
	    printf("i=%d (%s)\n", i, rapl_domain_names[i]);
	    printf("\terror opening core %d (%s) config %d: %s\n\n",
		   package_map[j], rapl_domain_names[i], config[i], strerror(errno));
	    return -1;
	  }
	}

	/* register the counter */
	register_measurement(mpii_info,
			     rapl_domain_names[i],
			     &rapl_plugin,
			     j,
			     i);
      }
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &start_date);

 return 0;
}


int mpi_rapl_start(struct mpii_info* mpii_info) {
  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &rapl_plugin) {
      m->counter_value = 0;
      m->period = 0;
    }
  }
  return 0;
}

int mpi_rapl_stop(struct mpii_info* mpii_info) {
  //struct rapl_measurement *m) {
  long long value;
  struct timespec stop_date;
  clock_gettime(CLOCK_MONOTONIC, &stop_date);

  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &rapl_plugin) {
      /* TODO: use one start date per counter ? */
      m->period = (stop_date.tv_sec-start_date.tv_sec)+((stop_date.tv_nsec-start_date.tv_nsec)/1e9);

      int package = m->device_id;
      int counter_id = m->counter_id;

      if (fd[counter_id][package] < 0) {
	fprintf(stderr, "[%s] Cannot read counter(device %d, counter %d)\n", m->plugin->plugin_name,
		package, counter_id);
      }
      read(fd[counter_id][package],&value,8);
	close(fd[counter_id][package]);
	m->counter_value += (double)value*scale[counter_id];

	if(value > MAX_VALUE) {
	  printf("Wow, that's a lot of joules ! (%lld)\n", value);
	  abort();
	}
    }
  }

  return 0;
}


void _rapl_init(void) __attribute__((constructor));
void _rapl_init(void){
  rapl_plugin.init = mpi_rapl_init;
  rapl_plugin.start_measurement = mpi_rapl_start;
  rapl_plugin.stop_measurement = mpi_rapl_stop;
  strcpy(rapl_plugin.plugin_name, "Perf_event");
  
  register_plugin(&rapl_plugin);
}
