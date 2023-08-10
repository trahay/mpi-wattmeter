#include <glob.h>
#include "mpii.h"

/************* Plugin management  **************/
struct mpii_info mpii_infos;

/* check if plugin_name is valid (ie. allowed by the -p option to mpi_wattmeter) */
static int _is_plugin_valid(const char* plugin_name) {
  if(strlen(mpii_infos.settings.plugin_list) == 0)
    /* no plugin specify -> use all the plugins */
    return 1;
  if(strcmp(mpii_infos.settings.plugin_list, "none") == 0)
    return 0;

  int is_valid = 0;
  
  char buffer[STRING_LENGTH];
  strncpy(buffer, mpii_infos.settings.plugin_list, STRING_LENGTH);
  char* saveptr;
  char* token = strtok_r(buffer, ",", &saveptr);
  while(token) {
    if(strcmp(token, plugin_name) == 0)
      is_valid = 1;
    token = strtok_r(NULL, ",", &saveptr);
  }
  return is_valid;
}

/* Try to register a plugin */
void register_plugin(struct measurement_plugin *plugin) {
  if(mpii_infos.nb_plugins > NB_PLUGINS_MAX) {
    fprintf(stderr, "Error: Too many plugins (%d)\n", mpii_infos.nb_plugins);
    exit(1);
  }

  if( _is_plugin_valid(plugin->plugin_name)) {
    MPII_PRINTF(debug_level_verbose, "register plugin '%s'\n", plugin->plugin_name);

    int plugin_index = mpii_infos.nb_plugins++;
    mpii_infos.plugins[plugin_index] = plugin;
  } else {
    MPII_PRINTF(debug_level_verbose, "Do not register plugin '%s' (authorized plugins: '%s')\n",
		plugin->plugin_name, mpii_infos.settings.plugin_list);
  }
}

/* Load a plugin library */
static void _load_plugin(const char* plugin_path) {
  void* handle = dlopen(plugin_path, RTLD_LAZY|RTLD_GLOBAL);
  if(!handle) {
    fprintf(stderr, "cannot load %s: %s\n", plugin_path, dlerror());
    abort();
  }
}

/* Load all the available plugins. Each plugin will register */
void load_plugins() {
  const char* base_libdir=INSTALL_LIBDIR;

  /* Search for all the plugins we can find */
  char plugin_pattern[STRING_LENGTH];
  snprintf(plugin_pattern, STRING_LENGTH, "%s/libmpi-wattmeter-*.so", base_libdir);

  glob_t globbuf;
  MPII_PRINTF(debug_level_verbose, "Searching for files that match %s\n", plugin_pattern);

  /* search for "libdir/libmpi-wattmeter-*.so" */
  if(glob(plugin_pattern, 0, NULL, &globbuf) == 0){
    MPII_PRINTF(debug_level_verbose, "found %lu matches\n", globbuf.gl_pathc);
    for(unsigned i=0; i<globbuf.gl_pathc; i++) {
      MPII_PRINTF(debug_level_verbose, "\t%s\n", globbuf.gl_pathv[i]);
      _load_plugin(globbuf.gl_pathv[i]);
    }
  }
  globfree(&globbuf);

  /* When we arrive here, all the allowed plugins have been registered.
   * Let's check if a user requested a plugin that was not found
   */
  char buffer[STRING_LENGTH];
  strncpy(buffer, mpii_infos.settings.plugin_list, STRING_LENGTH);
  char* saveptr;
  char* token = strtok_r(buffer, ",", &saveptr);
  while(token) {
    int is_found=0;
    for(int i=0; i<mpii_infos.nb_plugins; i++) {
      if(strcmp(token, mpii_infos.plugins[i]->plugin_name) == 0)
	is_found = 1;
    }
    if(!is_found) {
      MPII_PRINTF(debug_level_verbose, "[MPI-Wattmeter] Warning: Cannot find plugin '%s'\n", token);
    }
    token = strtok_r(NULL, ",", &saveptr);
  }

}
