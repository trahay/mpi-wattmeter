#include "mpii.h"
#include <inttypes.h>


static inline double joule_to_watthour(double joules) {
  return joules/3600;
}

static inline double joule_to_watt(double joules, double duration) {
  double watthour = joule_to_watthour(joules);
  double hour = duration/3600;
  return watthour/hour;
}

/* return the number of grams of CO2 emitted */
static inline double joule_to_co2(double joules) {
  /* In France electricity cost is around 36 gCO2/kWh
   * [https://www.rte-france.com/actualites/bilan-electrique-2021] */
  double kwatt = joule_to_watthour(joules)/1e3;
  return kwatt * 36;
}

static void _print_single_counter(const char *prefix, const char *source, double value, double duration);
static void _print_local_measurement(struct measurement *m, const char* prefix);
static void _print_statistics(const char* source, double total, double avg, double min, double max, double duration);

static void _init_measurement(struct measurement* m,
			      const char *counter_name,
			      struct measurement_plugin *plugin,
			      int device_id,
			      int counter_id);



void print_measurements() {
  static int already_passed = 0;
  if(already_passed) return;
  already_passed = 1;

  if(mpii_infos.is_local_master) {

    struct measurement measurements[mpii_infos.local_size][NB_COUNTERS_MAX];

    if(mpii_infos.mpi_mode) {
      /* TODO: we don't need to gather NB_COUNTER_MAX. We could gather nb_counters instead */
      MPI_Gather(&mpii_infos.measurements, sizeof(struct measurement)*NB_COUNTERS_MAX, MPI_BYTE,
		 measurements, sizeof(struct measurement)*NB_COUNTERS_MAX, MPI_BYTE,
		 0, mpii_infos.local_master_comm);

      /* we assume that all nodes have the same counters.
       * Thus, we change the plugin pointer to refer to the local plugin address
       */
      for(int i=0; i<mpii_infos.local_size; i++) {
	for(int j = 0; j<mpii_infos.nb_counters; j++)
	  measurements[i][j].plugin = mpii_infos.measurements[j].plugin;
      }
    } else {
      memcpy(measurements, &mpii_infos.measurements, sizeof(struct measurement)*NB_COUNTERS_MAX);
    }


    if(mpii_infos.local_rank == 0) {
      printf("============================\n");
      printf("MPI-Wattmeter summary\n");
      printf("============================\n");
      
      if(mpii_infos.settings.print_details) {
	for(int i=0; i<mpii_infos.local_size; i++) {
	  char prefix[128];
	  snprintf(prefix, 128, "%s:%d", get_rank_hostname(i), i);
	  for(int j=0; j<mpii_infos.nb_counters; j++) {
	    _print_local_measurement(&measurements[i][j], prefix);
	  }
	}
	printf("\n");
      }

      printf("Total:\n");
      for(int i=0; i<mpii_infos.nb_counters; i++) {
     	struct measurement min;
	struct measurement max;
	struct measurement total;
	struct measurement avg;
#define _INIT_MEASUREMENT(_m)						\
	_init_measurement(_m,						\
			  mpii_infos.measurements[i].counter_name,	\
			  mpii_infos.measurements[i].plugin,		\
			  mpii_infos.measurements[i].device_id,		\
			  mpii_infos.measurements[i].counter_id)
	_INIT_MEASUREMENT(&min);
	_INIT_MEASUREMENT(&max);
	_INIT_MEASUREMENT(&total);
	_INIT_MEASUREMENT(&avg);
      
	int nb_val = 0;      
	for(int rank=0; rank<mpii_infos.local_size; rank++) {
	  double counter_value = measurements[rank][i].counter_value;
	  double period = measurements[rank][i].period;

	  if(counter_value > 0) {
	    nb_val++;
	  
	    if(counter_value < min.counter_value || min.counter_value == 0)
	      min.counter_value = counter_value;

	    if(period < min.period || min.period == 0)
	      min.period = period;

	    if(counter_value > max.counter_value || max.counter_value == 0)
	      max.counter_value = counter_value;

	    if(period > max.period || max.period == 0)
	      max.period = period;

	    total.counter_value += counter_value;
	  }
	  total.period += period;
	}
	if(nb_val > 0 ) {
	  avg.counter_value =total.counter_value / nb_val;
	  avg.period = total.period / mpii_infos.local_size;
	  _print_statistics(avg.counter_name,
			   total.counter_value,
			   avg.counter_value,
			   min.counter_value,
			   max.counter_value,
			   avg.period);
	}
      }
    }
  }
}

static void _print_single_counter(const char *prefix, const char *source, double value, double duration) {
  static int first_time = 1;
  if(first_time) {
    printf("#%-14s\t%-10s",
	   "Node:rank",
	   "Source");
    if(mpii_infos.settings.print_joules)
      printf("\t%-10s", "joules");
    if(mpii_infos.settings.print_watthours)
      printf("\t%-10s", "Watt.hour");
    if(mpii_infos.settings.print_co2)
      printf("\t%-10s", "gCO2");
    if(mpii_infos.settings.print_watt)
      printf("\t%-10s", "Watt");

    printf("\n");
    first_time = 0;
  }
  printf("%-15s\t%-10s", prefix, source);
  if(mpii_infos.settings.print_joules)
    printf("\t%-10lf", value);
  if(mpii_infos.settings.print_watthours)
    printf("\t%-10lf", joule_to_watthour(value));
  if(mpii_infos.settings.print_co2)
    printf("\t%-10lf", joule_to_co2(value));
  if(mpii_infos.settings.print_watt)
    printf("\t%-10lf", joule_to_watt(value, duration));
  printf("\n");
}

static void _print_local_measurement(struct measurement *m, const char* prefix) {
  _print_single_counter(prefix, m->counter_name, m->counter_value, m->period);
  if(m->counter_value > MAX_VALUE) {
    printf("Wow, that's a lot of joules ! (%"PRIu64")\n", (uint64_t)m->counter_value);
    abort();
  }
}

static void _print_statistics(const char* source, double total, double avg, double min, double max, double duration) {
  static int first_time = 1;
  if(first_time) {
    printf("#%-14s", "Source");
    if(mpii_infos.settings.print_joules)
      printf("\t%-10s\t%-10s\t%-10s\t%-10s",
	     "total(j)",
	     "avg(j)",
	     "min(j)",
	     "max(j)");

    if(mpii_infos.settings.print_watthours)
      printf("\t%-10s\t%-10s\t%-10s\t%-10s",
	     "total(W.h)",
	     "avg(W.h)",
	     "min(W.h)",
	     "max(W.h)");

    if(mpii_infos.settings.print_co2)
      printf("\t%-10s\t%-10s\t%-10s\t%-10s",
	     "total(gCO2)",
	     "avg(gCO2)",
	     "min(gCO2)",
	     "max(gCO2)");

    if(mpii_infos.settings.print_watt)
      printf("\t%-10s\t%-10s\t%-10s",
	     "avg(W)",
	     "min(W)",
	     "max(W)");

    printf("\n");
    first_time = 0;
  }
  printf("%-15s", source);

  if(mpii_infos.settings.print_joules)
    printf("\t%-10lf\t%-10lf\t%-10lf\t%-10lf",
	   total,
	   avg,
	   min,
	   max);

  if(mpii_infos.settings.print_watthours)
    printf("\t%-10lf\t%-10lf\t%-10lf\t%-10lf",
	   joule_to_watthour(total),
	   joule_to_watthour(avg),
	   joule_to_watthour(min),
	   joule_to_watthour(max));

  if(mpii_infos.settings.print_co2)
    printf("\t%-10lf\t%-10lf\t%-10lf\t%-10lf",
	   joule_to_co2(total),
	   joule_to_co2(avg),
	   joule_to_co2(min),
	   joule_to_co2(max));

  if(mpii_infos.settings.print_watt)
    printf("\t%-10lf\t%-10lf\t%-10lf",
	   joule_to_watt(avg, duration),
	   joule_to_watt(min, duration),
	   joule_to_watt(max, duration));
  printf("\n");
}


void start_measurements() {

  for(int i=0; i<mpii_infos.nb_plugins; i++) {
    if(mpii_infos.plugins[i]->init(&mpii_infos)) {
      fprintf(stderr, "Initializing plugin %s failed.\n", mpii_infos.plugins[i]->plugin_name);
      exit(1);
    }
  }

  for(int i=0; i<mpii_infos.nb_plugins; i++) {
    if(mpii_infos.plugins[i]->start_measurement(&mpii_infos)) {
      fprintf(stderr, "[%s] start_measurement failed.\n", mpii_infos.plugins[i]->plugin_name);
      exit(1);
    }
  }
}


void stop_measurements() {
  if(mpii_infos.is_local_master) {

    for(int i=0; i<mpii_infos.nb_plugins; i++) {
      if(mpii_infos.plugins[i]->stop_measurement(&mpii_infos)) {
	fprintf(stderr, "[%s] stop_measurement failed.\n", mpii_infos.plugins[i]->plugin_name);
	exit(1);
      }      
    }

#if 0
    for(int i=0; i<mpii_infos.nb_counters; i++) {
      printf("%d - %s (dev %d, cpt %d): %lf\n", i, mpii_infos.measurements[i].counter_name,
	     mpii_infos.measurements[i].device_id,  mpii_infos.measurements[i].counter_id,
	     mpii_infos.measurements[i].counter_value);
    }
#endif
  }
}


static void _init_measurement(struct measurement* m,
			     const char *counter_name,
			     struct measurement_plugin *plugin,
			     int device_id,
			     int counter_id) {
  strncpy(m->counter_name, counter_name, 128);
  m->plugin = plugin;
  m->device_id = device_id;
  m->counter_id = counter_id;
  m->counter_value= 0;
  m->period = 0;
}

void register_measurement(struct mpii_info* mpii_info,
			  const char *counter_name,
			  struct measurement_plugin *plugin,
			  int device_id,
			  int counter_id) {
  if(mpii_infos.nb_counters > NB_COUNTERS_MAX) {
    fprintf(stderr, "Error: Too many counters (%d)\n", mpii_infos.nb_counters);
    exit(1);
  }

  int i = mpii_infos.nb_counters++;
  _init_measurement(&mpii_info->measurements[i],
		    counter_name,
		    plugin,
		    device_id,
		    counter_id);
}

void register_plugin(struct measurement_plugin *plugin) {
  if(mpii_infos.nb_plugins > NB_PLUGINS_MAX) {
    fprintf(stderr, "Error: Too many plugins (%d)\n", mpii_infos.nb_plugins);
    exit(1);
  }

  int plugin_index = mpii_infos.nb_plugins++;
  mpii_infos.plugins[plugin_index] = plugin;
}
