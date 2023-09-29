#include "mpii.h"

#include <rocm_smi/rocm_smi.h>
#include <time.h>

extern struct measurement_plugin rocm_plugin;

struct rocm_counter {
  char device_name[STRING_LENGTH];
  //  rocmDevice_t device;
  uint64_t gpu_energy;
  int device_id;
  //  int subdevice_id;
};

static unsigned nb_counters;
static struct rocm_counter *counters = NULL;

static struct timespec start_date;



static const char* rsmi_error_string(rsmi_status_t s) {
  switch(s) {
  case RSMI_STATUS_SUCCESS:
    return "Operation was successful";
    break;
  case RSMI_STATUS_INVALID_ARGS:
    return "Passed in arguments are not valid";
    break;
  case RSMI_STATUS_NOT_SUPPORTED:
    return "The requested information or action is not available for the given input, on the given system";
      break;
  case RSMI_STATUS_FILE_ERROR:
    return "Problem accessing a file.";
      break;
  case RSMI_STATUS_PERMISSION:
    return "Permission denied/EACCESS file error.";
      break;
  case RSMI_STATUS_OUT_OF_RESOURCES:
    return "Unable to acquire memory or other resource";
      break;
  case RSMI_STATUS_INTERNAL_EXCEPTION:
    return "An internal exception was caught";
    break;
  case RSMI_STATUS_INPUT_OUT_OF_BOUNDS:
    return "The provided input is out of allowable or safe range";
      break;
  case RSMI_STATUS_INIT_ERROR:
    return "An error occurred when rsmi initializing internal data structures";
      break;
  case RSMI_STATUS_NOT_YET_IMPLEMENTED:
    return "The requested function has not yet been implemented in the current system for the current devices";
      break;
  case RSMI_STATUS_NOT_FOUND:
    return "An item was searched for but not found";
      break;
  case RSMI_STATUS_INSUFFICIENT_SIZE:
    return "Not enough resources were available for the operation";
      break;
  case RSMI_STATUS_INTERRUPT:
    return "An interrupt occurred during execution of function";
      break;
  case RSMI_STATUS_UNEXPECTED_SIZE:
    return "An unexpected amount of data was read";
      break;
  case RSMI_STATUS_NO_DATA:
    return "No data was found for a given input";
      break;
  case RSMI_STATUS_UNEXPECTED_DATA:
    return "The data read or provided to function is not what was expected";
      break;
  case RSMI_STATUS_BUSY:
    return "A resource or mutex could not be acquired because it is already being used";
      break;
  case RSMI_STATUS_REFCOUNT_OVERFLOW:
    return "An internal reference counter exceeded INT32_MAX";
      break;
  case RSMI_STATUS_SETTING_UNAVAILABLE:
    return "Requested setting is unavailable for the current device";
      break;
  case RSMI_STATUS_AMDGPU_RESTART_ERR:
    return "Could not successfully restart the amdgpu driver";
      break;

  case RSMI_STATUS_UNKNOWN_ERROR:
    return "An unknown error occurred";
    break;
  }
  return "An unknown error occurred";
}

#if 0
static struct rocm_counter * _find_counter(int device_id, int subdevice_id) {
  for(unsigned i = 0; i<nb_counters; i++) {
    if(counters[i].device_id == device_id &&
       counters[i].subdevice_id == subdevice_id)
      return &counters[i];
  }
  return NULL;
}
  
#endif

int mpi_rocm_init(struct mpii_info *mpii_info) {
#if 1

  rsmi_status_t result;
  MPII_PRINTF(debug_level_verbose, "[MPI-Wattmeter::ROCM] Initializing plugin.\n");

  result = rsmi_init(0);
  if (result != RSMI_STATUS_SUCCESS) {
    fprintf(stderr, "rsmi_init failed: %s\n", rsmi_error_string(result));
    return -1;
  }
  uint32_t detected_devices;
  result = rsmi_num_monitor_devices(&detected_devices);
  if (result != RSMI_STATUS_SUCCESS) {
    fprintf(stderr, "rsmi_num_monito_devices failed: %s\n", rsmi_error_string(result));
    rsmi_shut_down();
    return -1;
  }

  nb_counters = 0;
  for(unsigned i=0; i<detected_devices; i++) {
    /* check if the GPU supports rsmi_dev_energy_count_get */

    uint64_t power;
    float counter_resolution;
    uint64_t timestamp;
    result = rsmi_dev_energy_count_get(i,
				       &power,
				       &counter_resolution,
				       &timestamp);

    if (result != RSMI_STATUS_SUCCESS) {
      fprintf(stderr, "rsmi_dev_energy_count: failed to get GPU energy consumtion on GPU %d: %s\n", i, rsmi_error_string(result));
      continue;
    }


    int counter_index = nb_counters++;
    counters = realloc(counters, sizeof(struct rocm_counter)*nb_counters);
    counters[counter_index].device_id = i;

    size_t len = STRING_LENGTH;
    result = rsmi_dev_name_get(i, counters[counter_index].device_name, len);
    if (result != RSMI_STATUS_SUCCESS) {
      fprintf(stderr, "rsmi_dev_name_get failed for GPU %d: %s\n", i, rsmi_error_string(result));
      continue;
    }

    MPII_PRINTF(debug_level_normal, "[MPI-Wattmeter::RSMI] Found counter %s.\n", counters[counter_index].device_name);

    /* register the counter */
    register_measurement(mpii_info,
			 counters[counter_index].device_name,
			 &rocm_plugin,
			 counters[counter_index].device_id,
			 -1);	/* TODO: is it possible to collect several energy consumption sources ? */
  }

  if (nb_counters < 1) {
    MPII_PRINTF(debug_level_normal, "[MPI-Wattmeter::RSMI] No supported GPU detected.\n");
    rsmi_shut_down();
    return -1;
  }

  MPII_PRINTF(debug_level_normal, "[MPI-Wattmeter::RSMI] Found %d counters.\n", nb_counters);

#else
  rocmReturn_t result;
  MPII_PRINTF(debug_level_verbose, "[MPI-Wattmeter::ROCM] Initializing plugin.\n");

  result = rocmInit();
  if (result != ROCM_SUCCESS) {
    fprintf(stderr, "rocmInit failed: %s\n", rocmErrorString(result));
    return -1;
  }
  unsigned detected_devices;
  result = rocmDeviceGetCount(&detected_devices);
  if (result != ROCM_SUCCESS) {
    fprintf(stderr, "rocmDeviceGetCount failed: %s\n", rocmErrorString(result));
    rocmShutdown();
    return -1;
  }

  nb_counters = 0;
  for(unsigned i=0; i<detected_devices; i++) {
    /* check if the GPU supports GetTotalEnergyConsumption */
    rocmDevice_t gpu;
    result = rocmDeviceGetHandleByIndex(i, &gpu);
    if (result != ROCM_SUCCESS) {
      fprintf(stderr, "rocmDeviceGetHandleByIndex failed for GPU %d: %s\n", i, rocmErrorString(result));
      continue;
    }
    unsigned long long energy;
    result = rocmDeviceGetTotalEnergyConsumption(gpu, &energy);
    if (result != ROCM_SUCCESS) {
      fprintf(stderr, "rocmDeviceGetTotalEnergyConsumption: failed to get GPU energy consumtion on GPU %d: %s\n", i, rocmErrorString(result));
      continue;
    }


    int counter_index = nb_counters++;
    counters = realloc(counters, sizeof(struct rocm_counter)*nb_counters);
    counters[counter_index].device_id = i;
    counters[counter_index].subdevice_id = -1;
    
    result = rocmDeviceGetHandleByIndex(i, &counters[counter_index].device);
    if (result != ROCM_SUCCESS) {
      fprintf(stderr, "rocmDeviceGetHandleByIndex failed for GPU %d: %s\n", i, rocmErrorString(result));
      rsmi_shut_down();
      return -1;
    }

    result = rocmDeviceGetName(counters[counter_index].device, counters[counter_index].device_name,
			       STRING_LENGTH);
    if (result != ROCM_SUCCESS) {
      fprintf(stderr, "rocmDeviceGetName failed for GPU %d: %s\n", i, rocmErrorString(result));
      return -1;
    }

    MPII_PRINTF(debug_level_normal, "[MPI-Wattmeter::ROCM] Found counter %s.\n", counters[counter_index].device_name);

    /* register the counter */
    register_measurement(mpii_info,
			 counters[counter_index].device_name,
			 &rocm_plugin,
			 counters[counter_index].device_id,
			 -1);	/* TODO: is it possible to collect several energy consumption sources ? */
  }

  if (nb_counters < 1) {
    MPII_PRINTF(debug_level_normal, "[MPI-Wattmeter::ROCM] No supported GPU detected.\n");
    rocmShutdown();
    return -1;
  }

  MPII_PRINTF(debug_level_normal, "[MPI-Wattmeter::ROCM] Found %d counters.\n", nb_counters);
#endif
  return 0;
}


/* Start ROCM measurement */

int mpi_rocm_start(struct mpii_info* mpii_info) {
#if 1
  //  

  clock_gettime(CLOCK_MONOTONIC, &start_date);

  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &rocm_plugin) {

      struct rocm_counter * c = &counters[m->device_id];
      if(c) {
	float counter_resolution = 0;
	uint64_t timestamp = 0;
	rsmi_status_t result = rsmi_dev_energy_count_get(c->device_id,
						    &c->gpu_energy,
						    &counter_resolution,
						    &timestamp);

	if (result != RSMI_STATUS_SUCCESS) {
	  fprintf(stderr, "rsmi_dev_energy_count_get: failed to get GPU energy consumtion: %s\n",
		  rsmi_error_string(result));
	  return -1;
	}
      }
    }
  }
#else
  clock_gettime(CLOCK_MONOTONIC, &start_date);

  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &rocm_plugin) {

      struct rocm_counter * c = _find_counter(m->device_id, m->counter_id);
      if(c) {
	rocmReturn_t result = rocmDeviceGetTotalEnergyConsumption(c->device, &c->gpu_energy);
	if (result != ROCM_SUCCESS) {
	  fprintf(stderr, "rocmDeviceGetTotalEnergyConsumption: failed to get GPU energy consumtion: %s\n", rocmErrorString(result));
	  return -1;
	}
      }
    }
  }
#endif
  return 0;  
}


/* Stop ROCM measurement */
int mpi_rocm_stop(struct mpii_info* mpii_info) {
  #if 1
  static struct timespec stop_date;
  clock_gettime(CLOCK_MONOTONIC, &stop_date);

  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &rocm_plugin) {

      struct rocm_counter * c = &counters[m->device_id];
      if(c) {
	m->period = (stop_date.tv_sec-start_date.tv_sec)+((stop_date.tv_nsec-start_date.tv_nsec)/1e9);

	uint64_t energy = 0;
	float counter_resolution = 0;
	uint64_t timestamp = 0;
	rsmi_status_t result = rsmi_dev_energy_count_get(c->device_id,
						    &energy,
						    &counter_resolution,
						    &timestamp);

	if (result != RSMI_STATUS_SUCCESS) {
	  fprintf(stderr, "rsmi_dev_energy_count_get: failed to get GPU energy consumtion: %s\n",
		  rsmi_error_string(result));
	  return -1;
	}

	m->counter_value = (energy - c->gpu_energy)/1e3; /* TODO: take counter_resolution into account */
	c->gpu_energy = energy;

      }
    }
  }

#else
  
  
  static struct timespec stop_date;
  clock_gettime(CLOCK_MONOTONIC, &stop_date);

  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &rocm_plugin) {
      struct rocm_counter * c = _find_counter(m->device_id, m->counter_id);

      if(c) {
	m->period = (stop_date.tv_sec-start_date.tv_sec)+((stop_date.tv_nsec-start_date.tv_nsec)/1e9);

	unsigned long long energy;
	rocmReturn_t result = rocmDeviceGetTotalEnergyConsumption(c->device, &energy);
	if (result != ROCM_SUCCESS) {
	  fprintf(stderr, "rocmDeviceGetTotalEnergyConsumption: failed to get GPU energy consumtion: %s\n", rocmErrorString(result));
	  return -1;
	}
	m->counter_value = (energy - c->gpu_energy)/1e3;
	c->gpu_energy = energy;
      }
    }
  }
#endif
  return 0;  
}


struct measurement_plugin rocm_plugin = {
  .init = mpi_rocm_init,
  .start_measurement = mpi_rocm_start,
  .stop_measurement = mpi_rocm_stop,
  .plugin_name= "ROCM",
  .plugin_description= "Use ROCM to measure the power consumption of AMD GPUs",
};

void _rocm_init(void) __attribute__((constructor));
void _rocm_init(void){
    
  register_plugin(&rocm_plugin);
}
