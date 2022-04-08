#include "mpii.h"

#if HAVE_NVML

#include <nvml.h>

static nvmlDevice_t *nvidia_gpu = NULL;
static unsigned dev_count=0;
static char **device_names=NULL;
static unsigned long long *gpu_energy = NULL;

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
    if (result == NVML_SUCCESS) {
      printf("GPU %u: %s\n", i, device_names[i]);
    } else {
      fprintf(stderr, "dwm-status: failed to get GPU 0 name: %s\n", nvmlErrorString(result));
      return -1;
    }
  }
  return dev_count;
}

/* Start NVML measurement */
int mpi_nvml_start(struct nvidia_measurement* m) {
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

static void init_measurement(struct nvidia_measurement *m) {
  m->energy = 0;
  m->device_name[0]='\0';
}

void print_gpu_measurements(struct nvidia_measurement* m, int local_size) {
  struct nvidia_measurement min;
  struct nvidia_measurement max;
  struct nvidia_measurement total;
  struct nvidia_measurement avg;
  init_measurement(&min);
  init_measurement(&max);
  init_measurement(&total);
  init_measurement(&avg);
  int total_gpus = 0;
  for(unsigned i=0; i<dev_count; i++) {
    for(int rank = 0; rank<local_size; rank++) {
      struct nvidia_measurement *cur = &m[(rank*dev_count) + i];
      if(min.energy < cur->energy || min.energy == 0)
	min.energy = cur->energy;

      if(max.energy > cur->energy || max.energy == 0)
	max.energy = cur->energy;

      total.energy += cur->energy;
      total_gpus++;
    }
  }

  if(total_gpus > 0 ) {
    avg.energy=total.energy / total_gpus;
    printf("[Total]\t\tGPU Energy Consumed (total/avg/min/max): %lf/%lf/%lf/%lf %s (%lf/%lf/%lf/%lf watts.hour)\n",
	   total.energy,
	   avg.energy,
	   min.energy,
	   max.energy,
	   "Joules",
	   joules_to_watthour(total.energy),
	   joules_to_watthour(avg.energy),
	   joules_to_watthour(min.energy),
	   joules_to_watthour(max.energy)
	   );
  }

}

#else
int mpi_nvml_init() {
  return 0;
}

int mpi_nvml_start(struct nvidia_measurement* m) {
  return 0;
}

int mpi_nvml_stop(struct nvidia_measurement* m) {
  return 0;
}
#endif
