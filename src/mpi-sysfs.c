#include "mpii.h"

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

struct measurement_plugin sysfs_plugin;

struct counter_data {
  int fd;
  uint64_t energy_data;
  int device_id;
  int subdevice_id;
};

static int nb_counters = 0;
static struct counter_data* counters = NULL;

static struct timespec start_date;

static char *name_base = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl:%d/%s";
static char *name_sub = "/sys/devices/virtual/powercap/intel-rapl/intel-rapl:%d/intel-rapl:%d:%d/%s";

static uint64_t _read_counter(int counter_index) {
  if(counter_index > nb_counters) {
    fprintf(stderr, "Sysfs: Invalid counter id: %d (nb_counters=%d)\n", counter_index, nb_counters);
    abort();
  }

  char energy_str[128];
  if(pread(counters[counter_index].fd, energy_str, 128, 0) < 0) {
    fprintf(stderr, "Sysfs: reading counter %d failed: %s\n", counter_index, strerror(errno));
    abort();
  }
  counters[counter_index].energy_data = strtoull(energy_str, NULL, 10);
  return counters[counter_index].energy_data;
}

static double _read_counter_from_id(int device_id, int subdevice_id) {
  for(int i=0; i<nb_counters; i++) {
    if(counters[i].device_id == device_id &&
       counters[i].subdevice_id == subdevice_id) {

      uint64_t prev_energy = counters[i].energy_data;
      uint64_t new_energy = _read_counter(i);

      uint64_t delta_ujoules = new_energy - prev_energy;
      double delta_joules = ((double)delta_ujoules)/1e6;
      return delta_joules;
    }
  }
  return 0;
}

static int _read_file(const char* filename, void* buffer, size_t buffer_size) {
  int fd = open(filename, O_RDONLY);
  if(fd < 0) {
    return 0;
  }

  if(read(fd, buffer, buffer_size)<0) {
    close(fd);
    return 0;
  }

  close(fd);
  return 1;
}

int register_counter(struct mpii_info *mpii_info, int device_id, int subdevice_id) {
  char name_filename[STRING_LENGTH];
  char energy_filename[STRING_LENGTH];

  if(subdevice_id < 0) {
    snprintf(name_filename, STRING_LENGTH, name_base, device_id, "name");
    snprintf(energy_filename, STRING_LENGTH, name_base, device_id, "energy_uj");
  } else {
    snprintf(name_filename, STRING_LENGTH, name_sub, device_id, device_id, subdevice_id, "name");
    snprintf(energy_filename, STRING_LENGTH, name_sub, device_id, device_id, subdevice_id, "energy_uj");
  }

  char counter_name[128];
  if(! _read_file(name_filename, counter_name, 128)) {
    /* counter is not readable/does not exist */
    return 0;
  }

  /* remove any trailing newline */
  counter_name[strcspn(counter_name, "\n")] = 0;

  /* make sure the energy file is readable */
  char energy[128];
  if(! _read_file(energy_filename, energy, 128)) {
    /* counter is not readable/does not exist */
    return 0;
  }

  /* Register this counter */
  int counter_id = nb_counters++;
  counters = realloc(counters, sizeof(struct counter_data) * nb_counters+1);
  counters[counter_id].fd = open(energy_filename, O_RDONLY);
  if(counters[counter_id].fd < 0)
    return 0;

  counters[counter_id].device_id = device_id;
  counters[counter_id].subdevice_id = subdevice_id;
  _read_counter(nb_counters-1);

  register_measurement(mpii_info,
		       counter_name,
		       &sysfs_plugin,
		       device_id,
		       subdevice_id);
  return 1;
}

int mpi_sysfs_init(struct mpii_info *mpii_info) {
  nb_counters = 0;
  for(int i=0; ; i++) {
    if(register_counter(mpii_info, i, -1) == 0)
      break;

    /* search for sub counters (eg. core counters) */
    for(int j=0; ; j++) {
      if(register_counter(mpii_info, i, j) == 0)
	break;
    }
  }

  if(nb_counters == 0) {

    char name_filename[STRING_LENGTH];
    snprintf(name_filename, STRING_LENGTH, name_base, 0, "name");
    fprintf(stderr, "Sysfs: Could not find any usable counter. Make sure %s is readable\n", name_filename);
    return -1;
  }
  clock_gettime(CLOCK_MONOTONIC, &start_date);

  return 0;
}


int mpi_sysfs_start(struct mpii_info* mpii_info) {
  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &sysfs_plugin) {

      _read_counter_from_id(m->device_id, m->counter_id);
      m->counter_value = 0;
      m->period = 0;
    }
  }
  return 0;
}

int mpi_sysfs_stop(struct mpii_info* mpii_info) {
  struct timespec stop_date;
  clock_gettime(CLOCK_MONOTONIC, &stop_date);

  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &sysfs_plugin) {

      m->period = (stop_date.tv_sec-start_date.tv_sec)+((stop_date.tv_nsec-start_date.tv_nsec)/1e9);
      double joules = _read_counter_from_id(m->device_id, m->counter_id);
      m->counter_value += joules;

      if(joules > MAX_VALUE) {
	printf("Wow, that's a lot of joules ! (%lf)\n", joules);
	abort();
      }
    }
  }

  return 0;
}


void _sysfs_init(void) __attribute__((constructor));
void _sysfs_init(void){
  sysfs_plugin.init = mpi_sysfs_init;
  sysfs_plugin.start_measurement = mpi_sysfs_start;
  sysfs_plugin.stop_measurement = mpi_sysfs_stop;
  strcpy(sysfs_plugin.plugin_name, "sysfs");
  
  register_plugin(&sysfs_plugin);
}
