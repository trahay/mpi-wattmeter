#include "mpii.h"

#if HAVE_NVML

#include <nvml.h>
#include <time.h>

struct measurement_plugin nvml_plugin;

static nvmlDevice_t *nvidia_gpu = NULL;
static unsigned dev_count=0;
static char **device_names=NULL;
static unsigned long long *gpu_energy = NULL;
static struct timespec start_date;

int mpi_nvml_init(struct mpii_info *mpii_info) {
  nvmlReturn_t result;

  result = nvmlInit();
  if (result != NVML_SUCCESS) {
    fprintf(stderr, "dwm-status: failed to initialize NVML: %s\n", nvmlErrorString(result));
    return -1;
  }
  int detected_devices;
  result = nvmlDeviceGetCount(&detected_devices);
  if (result != NVML_SUCCESS) {
    fprintf(stderr, "dwm-status: failed to get device count: %s\n", nvmlErrorString(result));
    nvmlShutdown();
    return -1;
  }

  dev_count = 0;
  for(unsigned i=0; i<detected_device; i++) {
    /* check if the GPU supports GetTotalEnergyConsumption */
    nvmlDevice_t gpu;
    result = nvmlDeviceGetHandleByIndex(i, &gpu);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "dwm-status: failed to get GPU %d: %s\n", i, nvmlErrorString(result));
      continue;
    }
    unsigned long long energy;
    result = nvmlDeviceGetTotalEnergyConsumption(gpu, &energy);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "nvmlDeviceGetTotalEnergyConsumption: failed to get GPU energy consumtion on GPU %d: %s\n", i, nvmlErrorString(result));
      continue;
    }
  }

  if (dev_count < 1) {
    fprintf(stderr, "[MPI-Wattmeter] No supported GPU detected\n");
    nvmlShutdown();
    return -1;
  }

  for(unsigned i=0; i<dev_count; i++) {
    char* device_name = alloca(sizeof(char) * NVML_DEVICE_NAME_BUFFER_SIZE);
    int device_id;
    result = nvmlDeviceGetHandleByIndex(i, &device_id);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "dwm-status: failed to get GPU 0: %s\n", nvmlErrorString(result));
      nvmlShutdown();
      return -1;
    }

    result = nvmlDeviceGetName(device_id, device_name, NVML_DEVICE_NAME_BUFFER_SIZE);
    if (result != NVML_SUCCESS) {
      fprintf(stderr, "dwm-status: failed to get GPU 0 name: %s\n", nvmlErrorString(result));
      return -1;
    }

    /* register the counter */
    register_measurement(mpii_info,
			 device_name,
			 &nvml_plugin,
			 device_id,
			 0);	/* TODO: is it possible to collect several energy consumption sources ? */

  }
  return dev_count;
}

/* Start NVML measurement */

int mpi_nvml_start(struct mpii_info* mpii_info) {
  clock_gettime(CLOCK_MONOTONIC, &start_date);

  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &perf_event_plugin) {

      nvmlReturn_t result = nvmlDeviceGetTotalEnergyConsumption(m->device_id, &m->counter_value);
      if (result != NVML_SUCCESS) {
	fprintf(stderr, "nvmlDeviceGetTotalEnergyConsumption: failed to get GPU energy consumtion: %s\n", nvmlErrorString(result));
	return -1;
      }
    }
  }
  return 0;  
}

/* Stop NVML measurement */
int mpi_nvml_stop(struct mpii_info* mpii_info) {
  long long value;
  static struct timespec stop_date;
  clock_gettime(CLOCK_MONOTONIC, &stop_date);

  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &perf_event_plugin) {
      m->period = (stop_date.tv_sec-start_date.tv_sec)+((stop_date.tv_nsec-start_date.tv_nsec)/1e9);

      unsigned long long energy;
      nvmlReturn_t result = nvmlDeviceGetTotalEnergyConsumption(m->device_id, &energy);
      if (result != NVML_SUCCESS) {
	fprintf(stderr, "nvmlDeviceGetTotalEnergyConsumption: failed to get GPU energy consumtion: %s\n", nvmlErrorString(result));
	return -1;
      }
      m->counter_value = (energy - m->counter_value)/1e3;
  }
  return 0;  
}


void _nvml_init(void) __attribute__((constructor));
void _nvml_init(void){
  nvml_plugin.init = mpi_nvml_init;
  nvml_plugin.start_measurement = mpi_nvml_start;
  nvml_plugin.stop_measurement = mpi_nvml_stop;
  strcpy(nvml_plugin.plugin_name, "NVML");
  
  register_plugin(&nvml_plugin);
}



#else
int mpi_nvml_init(struct mpii_info* mpii_info MAYBE_UNUSED) {
  return 0;
}

int mpi_nvml_start(struct mpii_info* mpii_info MAYBE_UNUSED) {
  return 0;
}

int mpi_nvml_stop(struct mpii_info* mpii_info MAYBE_UNUSED) {
  return 0;
}
#endif
