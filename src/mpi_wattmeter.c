#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <argp.h>
#include "mpii_config.h"

const char prefix[] = INSTALL_PREFIX;

const char *program_version = "mpi_wattmeter";
const char *program_bug_address = "";
static char doc[] = "mpi_wattmeter description";
static char args_doc[] = "target_application [TARGET OPTIONS]";
const char * argp_program_version="MPII dev";


// long name, key, arg, option flags, doc, group
// if key is negative or non printable, no short option
static struct argp_option options[] = {
	{0, 0, 0, 0, "Output options:"},
	{"verbose", 'v', 0, 0, "Produce verbose output" },
	{"details", 'd', 0, 0, "Print detailed measurements (default: OFF)" },
	{"joule", 'j', "yes|no", OPTION_ARG_OPTIONAL, "Print measurements in Joules (default: yes)" },
	{"watthour", 'W', "yes|no", OPTION_ARG_OPTIONAL, "Print measurements in Watt.hour (default: no)" },
	{"co2", 'c', "yes|no", OPTION_ARG_OPTIONAL, "Print measurements in CO2 grams (default: no)" },
	{"watt", 'w', "yes|no", OPTION_ARG_OPTIONAL, "Print measurements in Watts (default: yes)" },

	{0}
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  /* Get the input settings from argp_parse, which we
   * know is a pointer to our settings structure. */
  struct mpii_settings *settings = state->input;

  switch(key) {
  case 'v':
    settings->verbose = 1;
    break;
  case 'd':
    settings->print_details = 1;
    break;

  case 'j':
    if(arg && strcmp(arg, "no")==0) settings->print_joules = 0;
    else settings->print_joules = 1;
    break;
  case 'W':
    if(arg && strcmp(arg, "no")==0) settings->print_watthours = 0;
    else settings->print_watthours = 1;
    break;
  case 'c':
    if(arg && strcmp(arg, "no")==0) settings->print_co2 = 0;
    else settings->print_co2 = 1;
    break;
  case 'w':
    if(arg && strcmp(arg, "no")==0) settings->print_watt = 0;
    else settings->print_watt = 1;
    break;

  case ARGP_KEY_NO_ARGS:
    argp_usage(state);
    break;
  case ARGP_KEY_ARG:
  case ARGP_KEY_END:
    // nothing to do
    break;
  default:
    return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };


int main(int argc, char**argv) {
  struct mpii_settings settings;
  
  // Default values
  settings.verbose = SETTINGS_VERBOSE_DEFAULT;
  settings.print_details = SETTINGS_PRINT_DETAILS_DEFAULT;

  settings.print_joules =  SETTINGS_PRINT_JOULES_DEFAULT;
  settings.print_watthours = SETTINGS_PRINT_WATTHOURS_DEFAULT;
  settings.print_co2 = SETTINGS_PRINT_CO2_DEFAULT;
  settings.print_watt = SETTINGS_PRINT_WATT_DEFAULT;

  // first divide argv between mpii options and target file and
  // options optionnal todo : better target detection : it should be
  // possible to specify both --option=value and --option value, but
  // for now the latter is not interpreted as such
  int target_i = 1;
  while (target_i < argc && argv[target_i][0] == '-') target_i++;
  if (target_i == argc) {
    // there are no settings, either the user entered --help or
    // something like that, either we want to print usage anyway
    return argp_parse(&argp, argc, argv, 0, 0, &settings);
  }
  	
  char **target_argv = NULL;
  if (target_i < argc)
    target_argv = &(argv[target_i]);
  // we only want to parse what comes before target included
  argp_parse(&argp, target_i+1, argv, 0, 0, &settings);

  char ld_preload[STRING_LENGTH] = "";
  char *str;
  if ((str = getenv("LD_PRELOAD")) != NULL) {
    strncpy(ld_preload, str, STRING_LENGTH);
    strcat(ld_preload, ":");
  }
  strcat(ld_preload, prefix);
  strcat(ld_preload, "/");
  strcat(ld_preload, INSTALL_LIBDIR);
  strcat(ld_preload, "/libmpi-wattmeter.so");
	
  setenv("LD_PRELOAD", ld_preload, 1);

#define setenv_int(var, value, overwrite) do {	\
    char str[STRING_LENGTH];			\
    snprintf(str, STRING_LENGTH, "%d", value);	\
    setenv(var, str, overwrite);		\
  }while(0)
  
  setenv_int("MPII_VERBOSE", settings.verbose, 1);
  setenv_int("MPII_PRINT_DETAILS", settings.print_details, 1);
  setenv_int("MPII_PRINT_JOULES",   settings.print_joules, 1);
  setenv_int("MPII_PRINT_WATTHOURS",settings.print_watthours, 1);
  setenv_int("MPII_PRINT_CO2",      settings.print_co2, 1);
  setenv_int("MPII_PRINT_WATT",     settings.print_watt, 1);
  
  extern char** environ;
  int ret;
  if (target_argv != NULL) {
    ret  = execve(argv[target_i], target_argv, environ);
  } else {
    char *no_argv[] = {NULL};
    ret = execve(argv[target_i], no_argv, environ);
  }
  // execve failed
  fprintf(stderr, "Could not execve : %d - %s\n", errno, strerror(errno));
  return EXIT_FAILURE;
}
