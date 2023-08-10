#include "mpii.h"

#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_CPUS	1024
#define MAX_PACKAGES	16

const size_t BUFFER_SIZE = 4096;

extern struct measurement_plugin msr_plugin;

static int package_map[MAX_PACKAGES];
static int total_cores=0; // number of cores
static int total_packages=0; // number of packages (eg. sockets)

struct counter_data {
  int fd;
  unsigned int msr;
  double energy_units;
  char counter_name[STRING_LENGTH];

  double energy_data;

  int device_id;
  int subdevice_id;
};

static int nb_counters = 0;
static struct counter_data* counters = NULL;

static struct timespec start_date;

/* AMD Support */
#define MSR_AMD_RAPL_POWER_UNIT			0xc0010299

#define MSR_AMD_PKG_ENERGY_STATUS		0xc001029B
#define MSR_AMD_PP0_ENERGY_STATUS		0xc001029A



/* Intel support */

#define MSR_INTEL_RAPL_POWER_UNIT		0x606
/*
 * Platform specific RAPL Domains.
 * Note that PP1 RAPL Domain is supported on 062A only
 * And DRAM RAPL Domain is supported on 062D only
 */
/* Package RAPL Domain */
#define MSR_PKG_RAPL_POWER_LIMIT	0x610
#define MSR_INTEL_PKG_ENERGY_STATUS	0x611
#define MSR_PKG_PERF_STATUS		0x613
#define MSR_PKG_POWER_INFO		0x614

/* PP0 RAPL Domain */
#define MSR_PP0_POWER_LIMIT		0x638
#define MSR_INTEL_PP0_ENERGY_STATUS	0x639
#define MSR_PP0_POLICY			0x63A
#define MSR_PP0_PERF_STATUS		0x63B

/* PP1 RAPL Domain, may reflect to uncore devices */
#define MSR_PP1_POWER_LIMIT		0x640
#define MSR_PP1_ENERGY_STATUS		0x641
#define MSR_PP1_POLICY			0x642

/* DRAM RAPL Domain */
#define MSR_DRAM_POWER_LIMIT		0x618
#define MSR_DRAM_ENERGY_STATUS		0x619
#define MSR_DRAM_PERF_STATUS		0x61B
#define MSR_DRAM_POWER_INFO		0x61C

/* PSYS RAPL Domain */
#define MSR_PLATFORM_ENERGY_STATUS	0x64d

/* RAPL UNIT BITMASK */
#define POWER_UNIT_OFFSET	0
#define POWER_UNIT_MASK		0x0F

#define ENERGY_UNIT_OFFSET	0x08
#define ENERGY_UNIT_MASK	0x1F00

#define TIME_UNIT_OFFSET	0x10
#define TIME_UNIT_MASK		0xF000

#define CPU_VENDOR_INTEL	1
#define CPU_VENDOR_AMD		2

#define CPU_TIGERLAKE_MOBILE	140
#define CPU_TIGERLAKE           141
#define CPU_SANDYBRIDGE		42
#define CPU_SANDYBRIDGE_EP	45
#define CPU_IVYBRIDGE		58
#define CPU_IVYBRIDGE_EP	62
#define CPU_HASWELL		60
#define CPU_HASWELL_ULT		69
#define CPU_HASWELL_GT3E	70
#define CPU_HASWELL_EP		63
#define CPU_BROADWELL		61
#define CPU_BROADWELL_GT3E	71
#define CPU_BROADWELL_EP	79
#define CPU_BROADWELL_DE	86
#define CPU_SKYLAKE		78
#define CPU_SKYLAKE_HS		94
#define CPU_SKYLAKE_X		85
#define CPU_KNIGHTS_LANDING	87
#define CPU_KNIGHTS_MILL	133
#define CPU_KABYLAKE_MOBILE	142
#define CPU_KABYLAKE		158
#define CPU_ATOM_SILVERMONT	55
#define CPU_ATOM_AIRMONT	76
#define CPU_ATOM_MERRIFIELD	74
#define CPU_ATOM_MOOREFIELD	90
#define CPU_ATOM_GOLDMONT	92
#define CPU_ATOM_GEMINI_LAKE	122
#define CPU_ATOM_DENVERTON	95

#define CPU_AMD_FAM17H		0xc000

#define CPU_AMD_ZEN2_MEDOCINO     160
#define CPU_AMD_ZEN2_VANGOGH      144
#define CPU_AMD_ZEN2_MATISSE      113
#define CPU_AMD_ZEN2_LUCIENNE     104
#define CPU_AMD_ZEN2_RENOIR       96
#define CPU_AMD_ZEN2_XBOX         71
#define CPU_AMD_ZEN2_ROME         49
#define CPU_AMD_ZEN_DALI          32
#define CPU_AMD_ZEN_BANDEDKRESTEL 24
#define CPU_AMD_ZEN_RAVENRIDGE    17
#define CPU_AMD_ZEN_NAPLES        1

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

struct cpu_info {
  int cpu_vendor;
  int cpu_family;
  int cpu_model;
  char cpu_model_string[STRING_LENGTH];

  unsigned int msr_rapl_units;
  unsigned int msr_pkg_energy_status;
  unsigned int msr_pp0_energy_status;
  
  int pp0_avail;
  int pp1_avail;
  int dram_avail;
  int different_units;
  int psys_avail;
};


static struct cpu_info supported_cpus[] = {
  {CPU_VENDOR_INTEL, 6, CPU_TIGERLAKE_MOBILE, "Tigerlake", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 0, 1, 0, 1},
  {CPU_VENDOR_INTEL, 6, CPU_TIGERLAKE, "Tigerlake", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 0, 1, 0, 1},
  {CPU_VENDOR_INTEL, 6, CPU_SANDYBRIDGE, "Sandybridge", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 0, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_SANDYBRIDGE_EP, "Sandybridge-EP", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 0, 1, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_IVYBRIDGE, "Ivybridge", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 0, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_IVYBRIDGE_EP, "Ivybridge_EP", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 0, 1, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_HASWELL	, "Haswell", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 1, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_HASWELL_ULT	, "Haswell", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 1, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_HASWELL_GT3E, "Haswell", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 1, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_HASWELL_EP	, "Haswell-EP", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 0, 1, 1, 0},
  {CPU_VENDOR_INTEL, 6, CPU_BROADWELL	, "Broadwell", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 1, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_BROADWELL_GT3E, "Broadwell", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 1, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_BROADWELL_EP, "Broadwell-EP", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 0, 1, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_BROADWELL_DE, "Broadwell", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 1, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_SKYLAKE	, "Skylake", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 1, 0, 1},
  {CPU_VENDOR_INTEL, 6, CPU_SKYLAKE_HS	, "Skylake", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 1, 0, 1},
  {CPU_VENDOR_INTEL, 6, CPU_SKYLAKE_X	, "Skylake-X", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 0, 1, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_KNIGHTS_LANDING, "Knights_Landing", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 0, 0, 1, 1, 0},
  {CPU_VENDOR_INTEL, 6, CPU_KNIGHTS_MILL, "Knights_Mill", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 0, 0, 1, 1, 0},
  {CPU_VENDOR_INTEL, 6, CPU_KABYLAKE_MOBILE, "Kabylake",MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 1, 0, 1},
  {CPU_VENDOR_INTEL, 6, CPU_KABYLAKE	, "Kabylake", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 1, 1, 1, 0, 1},
  {CPU_VENDOR_INTEL, 6, CPU_ATOM_SILVERMONT, "Atom_Silvermont", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 0, 0, 0, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_ATOM_AIRMONT, "Atom_Airmont", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 0, 0, 0, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_ATOM_MERRIFIELD, "Atom_Merrifield", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 0, 0, 0, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_ATOM_MOOREFIELD, "Atom_Moorefield", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 0, 0, 0, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_ATOM_GOLDMONT, "Atom_Goldmont", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 0, 0, 0, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_ATOM_GEMINI_LAKE, "Atom_Gemini_Lake", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 0, 0, 0, 0, 0},
  {CPU_VENDOR_INTEL, 6, CPU_ATOM_DENVERTON, "Atom_Denverton", MSR_INTEL_RAPL_POWER_UNIT, MSR_INTEL_PKG_ENERGY_STATUS, MSR_INTEL_PP0_ENERGY_STATUS, 0, 0, 0, 0, 0},  
  {CPU_VENDOR_AMD, 23, CPU_AMD_ZEN2_MEDOCINO, "Zen2", MSR_AMD_RAPL_POWER_UNIT, MSR_AMD_PKG_ENERGY_STATUS, MSR_AMD_PP0_ENERGY_STATUS, 1, 0, 0, 0, 0},
  {CPU_VENDOR_AMD, 23, CPU_AMD_ZEN2_VANGOGH, "Zen2", MSR_AMD_RAPL_POWER_UNIT, MSR_AMD_PKG_ENERGY_STATUS, MSR_AMD_PP0_ENERGY_STATUS, 1, 0, 0, 0, 0},
  {CPU_VENDOR_AMD, 23, CPU_AMD_ZEN2_MATISSE, "Zen2", MSR_AMD_RAPL_POWER_UNIT, MSR_AMD_PKG_ENERGY_STATUS, MSR_AMD_PP0_ENERGY_STATUS, 1, 0, 0, 0, 0},
  {CPU_VENDOR_AMD, 23, CPU_AMD_ZEN2_LUCIENNE, "Zen2", MSR_AMD_RAPL_POWER_UNIT, MSR_AMD_PKG_ENERGY_STATUS, MSR_AMD_PP0_ENERGY_STATUS, 1, 0, 0, 0, 0},
  {CPU_VENDOR_AMD, 23, CPU_AMD_ZEN2_RENOIR, "Zen2", MSR_AMD_RAPL_POWER_UNIT, MSR_AMD_PKG_ENERGY_STATUS, MSR_AMD_PP0_ENERGY_STATUS, 1, 0, 0, 0, 0},
  {CPU_VENDOR_AMD, 23, CPU_AMD_ZEN2_XBOX, "Zen2", MSR_AMD_RAPL_POWER_UNIT, MSR_AMD_PKG_ENERGY_STATUS, MSR_AMD_PP0_ENERGY_STATUS, 1, 0, 0, 0, 0},
  {CPU_VENDOR_AMD, 23, CPU_AMD_ZEN2_ROME, "Zen2", MSR_AMD_RAPL_POWER_UNIT, MSR_AMD_PKG_ENERGY_STATUS, MSR_AMD_PP0_ENERGY_STATUS, 1, 0, 0, 0, 0},
  {CPU_VENDOR_AMD, 23, CPU_AMD_ZEN_DALI, "Zen", MSR_AMD_RAPL_POWER_UNIT, MSR_AMD_PKG_ENERGY_STATUS, MSR_AMD_PP0_ENERGY_STATUS, 1, 0, 0, 0, 0},
  {CPU_VENDOR_AMD, 23, CPU_AMD_ZEN_BANDEDKRESTEL, "Zen", MSR_AMD_RAPL_POWER_UNIT, MSR_AMD_PKG_ENERGY_STATUS, MSR_AMD_PP0_ENERGY_STATUS, 1, 0, 0, 0, 0},
  {CPU_VENDOR_AMD, 23, CPU_AMD_ZEN_RAVENRIDGE, "Zen", MSR_AMD_RAPL_POWER_UNIT, MSR_AMD_PKG_ENERGY_STATUS, MSR_AMD_PP0_ENERGY_STATUS, 1, 0, 0, 0, 0},
  {CPU_VENDOR_AMD, 23, CPU_AMD_ZEN_NAPLES, "Zen", MSR_AMD_RAPL_POWER_UNIT, MSR_AMD_PKG_ENERGY_STATUS, MSR_AMD_PP0_ENERGY_STATUS, 1, 0, 0, 0, 0},
};
static const int nb_supported_cpu = 39;

static struct cpu_info *current_cpu = NULL;



static int detect_cpu_model(void) {
  MPII_PRINTF(debug_level_verbose, "[MPI-Wattmeter::MSR] detecting CPUs.\n");
  FILE* fff=fopen("/proc/cpuinfo","r");
  if (fff==NULL) return -1;

  int cpu_vendor=0;
  int cpu_family=0;
  int cpu_model=0;

  while(1) {
    /* get the vendor_id, cpu family and model from /proc/cpuinfo */
    char buffer[BUFFER_SIZE];
    char* result=fgets(buffer,BUFFER_SIZE,fff);
    if (result==NULL) break;

    if (!strncmp(result,"vendor_id",8)) {
      char vendor_string[BUFFER_SIZE];
      sscanf(result,"%*s%*s%s", vendor_string);
      if (!strncmp(vendor_string,"GenuineIntel",12)) {cpu_vendor = CPU_VENDOR_INTEL; }
      if (!strncmp(vendor_string,"AuthenticAMD",12)) {cpu_vendor = CPU_VENDOR_AMD; }
    }
    if (!strncmp(result,"cpu family",10)) { sscanf(result,"%*s%*s%*s%d", &cpu_family); }
    if (!strncmp(result,"model",5)) { sscanf(result,"%*s%*s%d", &cpu_model);}
  }
  fclose(fff);

  for(int i=0; i<nb_supported_cpu;i++) {
    if((supported_cpus[i].cpu_vendor == cpu_vendor) &&
       (supported_cpus[i].cpu_family == cpu_family) &&
       (supported_cpus[i].cpu_model == cpu_model)) {
      current_cpu = &supported_cpus[i];
    }
  }

  if(!current_cpu) {
    MPII_PRINTF(debug_level_verbose,
		"[MPI-Wattmeter::MSR] Unsupported CPU: vendor_id: %d, family: %d, model: %d\n",
		cpu_vendor, cpu_family, cpu_model);
    return -1;
  }

  if ((cpu_vendor==CPU_VENDOR_INTEL) && (cpu_family!=6)) {
    MPII_WARN("[MPI-Wattmeter::MSR] Unsupported CPU family %d\n", cpu_family);
    return -1;
  }
  if ((cpu_vendor==CPU_VENDOR_AMD) && (cpu_family!=23)) {
    MPII_WARN("[MPI-Wattmeter::MSR] Unsupported CPU family %d\n", cpu_family);
    return -1;
  }
    
  MPII_PRINTF(debug_level_verbose,
	      "[MPI-Wattmeter::MSR] vendor_id: %d, family: %d, model: %d, arch: %s.\n",
	      current_cpu->cpu_vendor, current_cpu->cpu_family, current_cpu->cpu_model,
	      current_cpu->cpu_model_string);

  MPII_PRINTF(debug_level_verbose, "Processor type: %s\n", current_cpu->cpu_model_string);
  return 0;
}


static int open_msr(int core) {
  char msr_filename[BUFFER_SIZE];
  int fd;

  sprintf(msr_filename, "/dev/cpu/%d/msr", core);
  fd = open(msr_filename, O_RDONLY);
  if(fd < 0) {
    if ( errno == ENXIO ) {
      MPII_WARN("[MPI-Wattmeter::MSR] No CPU %d\n", core);
      return -1;
    } else if ( errno == EIO ) {
      MPII_WARN("[MPI-Wattmeter::MSR] CPU %d doesn't support MSRs\n", core);
      return -1;
    } else {
      MPII_WARN("[MPI-Wattmeter::MSR] Error Trying to open %s: %s\n",msr_filename, strerror(errno));
      return -1;
    }
  }
  return fd;
}

static long long read_msr(int fd, unsigned int which) {
  uint64_t data;

  if (pread(fd, &data, sizeof(data), which) != sizeof(data) ) {
    MPII_WARN("[MPI-Wattmeter::MSR] Error reading MSR %x: %s\n",which, strerror(errno));
    exit(127);
  }

  return (long long)data;
}





static int register_counter(struct mpii_info *mpii_info,
			    const char* counter_name,
			    int device_id,
			    int fd,
			    unsigned int msr,
			    double energy_units) {

  /* Register this counter */
  int counter_id = nb_counters++;
  counters = realloc(counters, sizeof(struct counter_data) * nb_counters+1);

  assert(fd > 0);
  counters[counter_id].fd = fd;
  counters[counter_id].msr = msr;
  counters[counter_id].energy_units = energy_units;
  strncpy(counters[counter_id].counter_name, counter_name, STRING_LENGTH);

  counters[counter_id].device_id = device_id;
  counters[counter_id].subdevice_id = counter_id;
  
  MPII_PRINTF(debug_level_normal, "[MPI-Wattmeter::MSR] Found counter %s.\n", counter_name);

  register_measurement(mpii_info,
		       counter_name,
		       &msr_plugin,
		       device_id,
		       counter_id);
  return 1;
}


static void initialize_msr() {
  assert(current_cpu != NULL);

  double power_units,time_units;
  long long result;
  
  for(int i=0; i<total_packages; i++) {
    double cpu_energy_units;
    char counter_name[STRING_LENGTH];

    MPII_PRINTF(debug_level_verbose, "[MPI-Wattmeter::MSR] Initialize package %d.\n", i);

    int fd = open_msr(package_map[i]);
    if(fd < 0)
      continue;
    /* Calculate the units used */
    result = read_msr(fd, current_cpu->msr_rapl_units);
    cpu_energy_units=pow(0.5,(double)((result>>8)&0x1f));

    /* Package energy */
    snprintf(counter_name, STRING_LENGTH, "Package%d", i);
    register_counter(&mpii_infos,
		     counter_name,
		     i,
		     fd,
		     current_cpu->msr_pkg_energy_status,
		     cpu_energy_units);
    
    /* PP0 energy */
    if (current_cpu->pp0_avail) {
      snprintf(counter_name, STRING_LENGTH, "PowerPlane0_%d(cores)", i);
      register_counter(&mpii_infos,
		       counter_name,
		       i,
		       fd,
		       current_cpu->msr_pp0_energy_status,
		       cpu_energy_units);
    }

    /* PP1 energy */
    /* not available on *Bridge-EP */
    if (current_cpu->pp1_avail) {
      snprintf(counter_name, STRING_LENGTH, "PowerPlane1_%d(on-core_GPU)", i);
      register_counter(&mpii_infos,
		       counter_name,
		       i,
		       fd,
		       MSR_PP1_ENERGY_STATUS,
		       cpu_energy_units);
    }


    /* Updated documentation (but not the Vol3B) says Haswell and	*/
    /* Broadwell have DRAM support too				*/
    if (current_cpu->dram_avail) {
      double dram_energy_units = cpu_energy_units;

      /* On Haswell EP and Knights Landing */
      /* The DRAM units differ from the CPU ones */
      if (current_cpu->different_units) {
	dram_energy_units = pow(0.5,(double)16);
      }

      snprintf(counter_name, STRING_LENGTH, "DRAM%d", i);
      register_counter(&mpii_infos,
		       counter_name,
		       i,
		       fd,
		       MSR_DRAM_ENERGY_STATUS,
		       dram_energy_units);
    }

    /* Skylake and newer for Psys				*/
    if (current_cpu->psys_avail) {
      snprintf(counter_name, STRING_LENGTH, "PSYS%d", i);
      register_counter(&mpii_infos,
		       counter_name,
		       i,
		       fd,
		       MSR_PLATFORM_ENERGY_STATUS,
		       cpu_energy_units);
    }

  }
}






static double _read_counter_from_id(int device_id, int subdevice_id) {
  for(int i=0; i<nb_counters; i++) {
    struct counter_data*cpt = &counters[i];
    if((cpt->device_id == device_id) &&
       (cpt->subdevice_id == subdevice_id)) {
      
      long long result = read_msr(cpt->fd, cpt->msr);
      cpt->energy_data = (double)result * cpt->energy_units;

      return cpt->energy_data;
    }
  }
}

int msr_init(struct mpii_info *mpii_info) {
  static int initialized = 0;
  if(initialized++)
    return 0;

  MPII_PRINTF(debug_level_verbose, "[MPI-Wattmeter::msr] Initializing plugin.\n");

  detect_packages();
  detect_cpu_model();
  initialize_msr();

  MPII_PRINTF(debug_level_normal,  "[MPI-Wattmeter::msr] Found %d counters.\n", nb_counters);
  return 0;
}


int msr_start(struct mpii_info *mpii_info) {
  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &msr_plugin) {
      m->counter_value = _read_counter_from_id(m->device_id, m->counter_id);
      m->period = 0;
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &start_date);
  return 0;
}

int msr_stop(struct mpii_info *mpii_info) {
  struct timespec stop_date;
  clock_gettime(CLOCK_MONOTONIC, &stop_date);

  for(int i=0; i<mpii_info->nb_counters; i++) {
    struct measurement *m = &mpii_info->measurements[i];
    if(m->plugin == &msr_plugin) {

      double prev_value = m->counter_value;
      double new_value = _read_counter_from_id(m->device_id, m->counter_id);

      m->counter_value = new_value - prev_value;
      m->period = (stop_date.tv_sec-start_date.tv_sec)+((stop_date.tv_nsec-start_date.tv_nsec)/1e9);
    }
  }
  return 0;
}

struct measurement_plugin msr_plugin = {
  .init = msr_init,
  .start_measurement = msr_start,
  .stop_measurement = msr_stop,
  .plugin_name = "msr",
  .plugin_description = "Use MSR to access RAPL meters",
};

void _msr_init(void) __attribute__((constructor));
void _msr_init(void){

  register_plugin(&msr_plugin);
}
