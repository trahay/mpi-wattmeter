#include "mpii.h"

#if HAVE_NVML

#include <nvml.h>
#include <time.h>

static nvmlDevice_t *nvidia_gpu = NULL;
static unsigned dev_count=0;
static char **device_names=NULL;
static unsigned long long *gpu_energy = NULL;
static struct timespec start_date;

int mpi_nvml_init() {
  nvmlReturn_t result;

  result = nvmlInit();
  if (result != NVML_SUCCESS) {
    fprintf(stderr, "dwm-status: failed to initialize NVML: %s\n", nvmlErrorString(result));
    return -1;
  }
  result = nvmlDeviceGetCount(&dev_count);
  if (result != NVML_SUCCESS) {
    fprintf(stderr, "dwm-status: failed to get device count: %s\n", nvmlErrorString(result));
    nvmlShutdown();
    return -1;
  }

  if (dev_count < 1) {
    nvmlShutdown();
    return -1;
  }

  nvidia_gpu = malloc(sizeof(nvmlDevice_t) * dev_count);
  device_names = malloc(sizeof(char*) * dev_count);
  gpu_energy = malloc(sizeof(unsigned long long) * dev_count);
  for(unsigned i=0; i<dev_count; i++) {
    device_names[i] = malloc(sizeof(char) * NVML_DEVICE_NAME_BUFFER_SIZE);

    result = nvmlDeviceGetHandleByIndex(i, &nvidia_gpu[i]);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "dwm-status: failed to get GPU 0: %s\n", nvmlErrorString(result));
      nvmlShutdown();
      return -1;
    }

    result = nvmlDeviceGetName(nvidia_gpu[i], device_names[i], NVML_DEVICE_NAME_BUFFER_SIZE);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "dwm-status: failed to get GPU 0 name: %s\n", nvmlErrorString(result));
      return -1;
    }
  }
  return dev_count;
}

/* Start NVML measurement */
int mpi_nvml_start(struct nvidia_measurement* m) {
  clock_gettime(CLOCK_MONOTONIC, &start_date);
  for(unsigned i=0; i<dev_count; i++) {
    nvmlReturn_t result = nvmlDeviceGetTotalEnergyConsumption(nvidia_gpu[i], &gpu_energy[i]);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "nvmlDeviceGetTotalEnergyConsumption: failed to get GPU energy consumtion: %s\n", nvmlErrorString(result));
      return -1;
    }
    strcpy(m[i].device_name, device_names[i]);
  }
  return 0;  
}

/* Stop NVML measurement */
int mpi_nvml_stop(struct nvidia_measurement* m) {
  static struct timespec stop_date;
  clock_gettime(CLOCK_MONOTONIC, &stop_date);
  m->period = (stop_date.tv_sec-start_date.tv_sec)+((stop_date.tv_nsec-start_date.tv_nsec)/1e9);

  for(unsigned i=0; i<dev_count; i++) {
    unsigned long long energy;
    nvmlReturn_t result = nvmlDeviceGetTotalEnergyConsumption(nvidia_gpu[i], &energy);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "nvmlDeviceGetTotalEnergyConsumption: failed to get GPU energy consumtion: %s\n", nvmlErrorString(result));
      return -1;
    }
    m[i].energy = (energy - gpu_energy[i])/1e3;
  }
  return 0;  
}


#else
int mpi_nvml_init() {
  return 0;
}

int mpi_nvml_start(struct nvidia_measurement* m MAYBE_UNUSED) {
  return 0;
}

int mpi_nvml_stop(struct nvidia_measurement* m MAYBE_UNUSED) {
  return 0;
}
#endif
