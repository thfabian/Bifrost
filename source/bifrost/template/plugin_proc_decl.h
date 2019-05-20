//
// Definition of procedures
//
#pragma region Plugin Procedure

#define BIFROST_PLUGIN_SETUP_PROC_NAME bifrost_PluginSetUp
#define BIFROST_PLUGIN_SETUP_PROC_NAME_STRING "bifrost_PluginSetUp"
#define BIFROST_PLUGIN_SETUP_PROC_TYPE BOOL (*)(LPVOID)
#define BIFROST_PLUGIN_SETUP_PROC_DECL                                     \
  extern "C" {                                                             \
  __declspec(dllexport) BOOL BIFROST_PLUGIN_SETUP_PROC_NAME(LPVOID param); \
  }
#define BIFROST_PLUGIN_SETUP_PROC_DEF BOOL BIFROST_PLUGIN_SETUP_PROC_NAME(LPVOID param)

#pragma endregion