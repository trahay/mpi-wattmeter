#include "mpii.h"
#include <inttypes.h>


static inline double joules_to_watthour(double joules) {
  return joules/3600;
}

/* return the number of grams of CO2 emitted */
static inline double joules_to_co2(double joules) {
  /* In France electricity cost is around 36 gCO2/kWh
   * [https://www.rte-france.com/actualites/bilan-electrique-2021] */
  double kwatt = joules_to_watthour(joules)/1e3;
  return kwatt * 36;
}

void print_local_rapl_measurement(struct rapl_measurement *m, const char* prefix);
void print_rapl_measurements(struct rapl_measurement *m, int nb);
void print_gpu_measurements(struct nvidia_measurement* m);
void print_local_gpu_measurement(struct nvidia_measurement* m, int rank);

void start_measurements() {
  mpi_rapl_init();

  mpii_infos.nb_gpus = mpi_nvml_init();
  mpii_infos.gpu_measurement = malloc(mpii_infos.nb_gpus * sizeof(struct nvidia_measurement));
  mpi_nvml_start(mpii_infos.gpu_measurement);

  mpi_rapl_start();
}


void stop_measurements() {

  if(mpii_infos.is_local_master) {
    mpi_rapl_stop(&mpii_infos.rapl_measurement);
    mpi_nvml_stop(mpii_infos.gpu_measurement);
  }
}


void print_measurements() {
  static int already_passed = 0;
  if(already_passed) return;
  already_passed = 1;

  if(mpii_infos.is_local_master) {

    struct rapl_measurement measurements[mpii_infos.local_size];
    struct nvidia_measurement nvidia_measurements[mpii_infos.local_size];

    if(mpii_infos.mpi_mode) {
      MPI_Gather(&mpii_infos.rapl_measurement, sizeof(struct rapl_measurement), MPI_BYTE,
		 measurements, sizeof(struct rapl_measurement), MPI_BYTE,
		 0, mpii_infos.local_master_comm);

      MPI_Gather(mpii_infos.gpu_measurement, sizeof(struct nvidia_measurement)*mpii_infos.nb_gpus, MPI_BYTE,
		 nvidia_measurements, sizeof(struct nvidia_measurement)*mpii_infos.nb_gpus, MPI_BYTE,
		 0, mpii_infos.local_master_comm);
    } else {
      memcpy(measurements, &mpii_infos.rapl_measurement, sizeof(struct rapl_measurement));
      memcpy(nvidia_measurements, mpii_infos.gpu_measurement, sizeof(struct nvidia_measurement));
    }


    if(mpii_infos.local_rank == 0) {
      printf("============================\n");
      printf("MPI-Wattmeter summary\n");
      printf("============================\n");
      
      if(mpii_infos.settings.print_details) {
	for(int i=0; i<mpii_infos.local_size; i++) {
	  char prefix[128];
	  snprintf(prefix, 128, "%s:%d", get_rank_hostname(i), i);
	  print_local_rapl_measurement(&measurements[i], prefix);
	  print_local_gpu_measurement(&nvidia_measurements[i], i);
	}
	printf("\n");
      }

      printf("Total:\n");
      print_rapl_measurements(measurements, mpii_infos.local_size);
      print_gpu_measurements(nvidia_measurements);
    }
  }
}


void print_single_counter(const char *prefix, const char *source, double value) {
  static int first_time = 1;
  if(first_time) {
    printf("#%-14s\t%-10s\t%-10s\t%-10s\t%-10s\n",
	   "Node:rank",
	   "Source",
	   "joules",
	   "Watt.hour",
	   "gCO2");
    first_time = 0;
  }
  printf("%-15s\t%-10s\t%-10lf\t%-10lf\t%-10lf\n",
	 prefix,
	 source,
	 value,
	 joules_to_watthour(value),
	 joules_to_co2(value));
}

void print_local_rapl_measurement(struct rapl_measurement *m, const char* prefix) {
  for(int i=0;i<NUM_RAPL_DOMAINS;i++) {
    if(m->counter_value[i] > 0 ) {
      print_single_counter(prefix, rapl_domain_names[i], m->counter_value[i]);
      if(m->counter_value[i] > MAX_VALUE) {
	printf("Wow, that's a lot of joules ! (%"PRIu64")\n", (uint64_t)m->counter_value[i]);
	abort();
      }
    }
  }
}

void print_local_gpu_measurement(struct nvidia_measurement *m, int rank) {

  for(int i=0;i<mpii_infos.nb_gpus;i++) {

    char prefix[32];
    snprintf(prefix, 32, "%s:GPU#%d", get_rank_hostname(rank), i);
    print_single_counter(prefix, "GPU", m[i].energy);
    if(m[i].energy > MAX_VALUE) {
      printf("Wow, that's a lot of joules ! (%"PRIu64")\n", (uint64_t)m[i].energy);
      abort();
    }
  }
}

void print_statistics(const char* source, double total, double avg, double min, double max) {
  static int first_time = 1;
  if(first_time) {
    printf("#%-14s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\t%-10s\n",
	   "Source",
	   "total(j)",
	   "avg(j)",
	   "min(j)",
	   "max(j)",
	   "total(W.h)",
	   "avg(W.h)",
	   "min(W.h)",
	   "max(W.h)",
	   "total(gCO2)",
	   "avg(gCO2)",
	   "min(gCO2)",
	   "max(gCO2)");
	  first_time = 0;
  }
  printf("%-15s\t%-10lf\t%-10lf\t%-10lf\t%-10lf\t%-10lf\t%-10lf\t%-10lf\t%-10lf\t%-10lf\t%-10lf\t%-10lf\t%-10lf\n",
	 source,
	 total,
	 avg,
	 min,
	 max,
	 joules_to_watthour(total),
	 joules_to_watthour(avg),
	 joules_to_watthour(min),
	 joules_to_watthour(max),
	 joules_to_co2(total),
	 joules_to_co2(avg),
	 joules_to_co2(min),
	 joules_to_co2(max)
	 );
}

static void init_rapl_measurement(struct rapl_measurement *m) {
  for(int i=0;i<NUM_RAPL_DOMAINS;i++) {
    m->counter_value[i] = 0;
  }
}

void print_rapl_measurements(struct rapl_measurement *m, int nb) {
  struct rapl_measurement min;
  struct rapl_measurement max;
  struct rapl_measurement total;
  struct rapl_measurement avg;
  init_rapl_measurement(&min);
  init_rapl_measurement(&max);
  init_rapl_measurement(&total);
  init_rapl_measurement(&avg);

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
      print_statistics(rapl_domain_names[j],
		       total.counter_value[j],
		       avg.counter_value[j],
		       min.counter_value[j],
		       max.counter_value[j]);
    }
  }
}

static void init_nvidia_measurement(struct nvidia_measurement *m) {
  m->energy = 0;
  m->device_name[0]='\0';
}


void print_gpu_measurements(struct nvidia_measurement* m) {
  struct nvidia_measurement min;
  struct nvidia_measurement max;
  struct nvidia_measurement total;
  struct nvidia_measurement avg;
  init_nvidia_measurement(&min);
  init_nvidia_measurement(&max);
  init_nvidia_measurement(&total);
  init_nvidia_measurement(&avg);
  int total_gpus = 0;
  for(int i=0; i<mpii_infos.nb_gpus; i++) {
    for(int rank = 0; rank<mpii_infos.local_size; rank++) {
      struct nvidia_measurement *cur = &m[(rank*mpii_infos.nb_gpus) + i];
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
    print_statistics("GPUs",
		     total.energy,
		     avg.energy,
		     min.energy,
		     max.energy);
  }
}
