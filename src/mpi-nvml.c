#include "mpii.h"

#include <nvml.h>
#include <time.h>

extern struct measurement_plugin nvml_plugin;

struct nvml_counter {
  char device_name[STRING_LENGTH];
  nvmlDevice_t device;
  unsigned long long gpu_energy;

  int device_id;
  int subdevice_id;
};

static unsigned nb_counters;
static struct nvml_counter *counters = NULL;

static struct timespec start_date;


static struct nvml_counter * _find_counter(int device_id, int subdevice_id) {
  for(unsigned i = 0; i<nb_counters; i++) {
    if(counters[i].device_id == device_id &&
       counters[i].subdevice_id == subdevice_id)
      return &counters[i];
  }
  return NULL;
}
  

int mpi_nvml_init(struct mpii_info *mpii_info) {
  nvmlReturn_t result;
  MPII_PRINTF(debug_level_verbose, "[MPI-Wattmeter::NVML] Initializing plugin.\n");

  result = nvmlInit();
  if (result != NVML_SUCCESS) {
    fprintf(stderr, "nvmlInit failed: %s\n", nvmlErrorString(result));
    return -1;
  }
  unsigned detected_devices;
  result = nvmlDeviceGetCount(&detected_devices);
  if (result != NVML_SUCCESS) {
    fprintf(stderr, "nvmlDeviceGetCount failed: %s\n", nvmlErrorString(result));
    nvmlShutdown();
    return -1;
  }

  nb_counters = 0;
  for(unsigned i=0; i<detected_devices; i++) {
    /* check if the GPU supports GetTotalEnergyConsumption */
    nvmlDevice_t gpu;
    result = nvmlDeviceGetHandleByIndex(i, &gpu);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "nvmlDeviceGetHandleByIndex failed for GPU %d: %s\n", i, nvmlErrorString(result));
      continue;
    }
    unsigned long long energy;
    result = nvmlDeviceGetTotalEnergyConsumption(gpu, &energy);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "nvmlDeviceGetTotalEnergyConsumption: failed to get GPU energy consumtion on GPU %d: %s\n", i, nvmlErrorString(result));
      continue;
    }


    int counter_index = nb_counters++;
    counters = realloc(counters, sizeof(struct nvml_counter)*nb_counters);
    counters[counter_index].device_id = i;
    counters[counter_index].subdevice_id = -1;
    
    result = nvmlDeviceGetHandleByIndex(i, &counters[counter_index].device);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "nvmlDeviceGetHandleByIndex failed for GPU %d: %s\n", i, nvmlErrorString(result));
      nvmlShutdown();
      return -1;
    }

    result = nvmlDeviceGetName(counters[counter_index].device, counters[counter_index].device_name,
			       STRING_LENGTH);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "nvmlDeviceGetName failed for GPU %d: %s\n", i, nvmlErrorString(result));
      return -1;
    }

    MPII_PRINTF(debug_level_normal, "[MPI-Wattmeter::NVML] Found counter %s.\n", counters[counter_index].device_name);

    /* register the counter */
    register_measurement(mpii_info,
			 counters[counter_index].device_name,
			 &nvml_plugin,
			 counters[counter_index].device_id,
			 -1);	/* TODO: is it possible to collect several energy consumption sources ? */
  }

  if (nb_counters < 1) {
    MPII_PRINTF(debug_level_normal, "[MPI-Wattmeter::NVML] No supported GPU detected.\n");
    nvmlShutdown();
    return -1;
  }

  MPII_PRINTF(debug_level_normal, "[MPI-Wattmeter::NVML] Found %d counters.\n", nb_counters);

  return 0;
}

/* Start NVML measurement */

int mpi_nvml_start(struct mpii_info* mpii_info) {
  clock_gettime(CLOCK_MONOTONIC, &start_date);

  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &nvml_plugin) {

      struct nvml_counter * c = _find_counter(m->device_id, m->counter_id);
      if(c) {
	nvmlReturn_t result = nvmlDeviceGetTotalEnergyConsumption(c->device, &c->gpu_energy);
	if (result != NVML_SUCCESS) {
	  fprintf(stderr, "nvmlDeviceGetTotalEnergyConsumption: failed to get GPU energy consumtion: %s\n", nvmlErrorString(result));
	  return -1;
	}
      }
    }
  }
  return 0;  
}


/* Stop NVML measurement */
int mpi_nvml_stop(struct mpii_info* mpii_info) {
  static struct timespec stop_date;
  clock_gettime(CLOCK_MONOTONIC, &stop_date);

  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &nvml_plugin) {
      struct nvml_counter * c = _find_counter(m->device_id, m->counter_id);

      if(c) {
	m->period = (stop_date.tv_sec-start_date.tv_sec)+((stop_date.tv_nsec-start_date.tv_nsec)/1e9);

	unsigned long long energy;
	nvmlReturn_t result = nvmlDeviceGetTotalEnergyConsumption(c->device, &energy);
	if (result != NVML_SUCCESS) {
	  fprintf(stderr, "nvmlDeviceGetTotalEnergyConsumption: failed to get GPU energy consumtion: %s\n", nvmlErrorString(result));
	  return -1;
	}
	m->counter_value = (energy - c->gpu_energy)/1e3;
	c->gpu_energy = energy;
      }
    }
  }
  return 0;  
}


struct measurement_plugin nvml_plugin = {
  .init = mpi_nvml_init,
  .start_measurement = mpi_nvml_start,
  .stop_measurement = mpi_nvml_stop,
  .plugin_name, "NVML",
  .plugin_description, "Use Nvidia Management Library (NVML) to measure the power consumption of GPUs",
};

void _nvml_init(void) __attribute__((constructor));
void _nvml_init(void){
    
  register_plugin(&nvml_plugin);
}
