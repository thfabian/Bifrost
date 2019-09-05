#pragma region Plugin Procedure

#define BIFROST_PLUGIN_SETUP_PROC_NAME bifrost_PluginSetUp
#define BIFROST_PLUGIN_SETUP_PROC_NAME_STRING "bifrost_PluginSetUp"
#define BIFROST_PLUGIN_SETUP_PROC_TYPE int (*)(void*) noexcept(false)
#define BIFROST_PLUGIN_SETUP_PROC_DECL                                                   \
  extern "C" {                                                                           \
  __declspec(dllexport) int BIFROST_PLUGIN_SETUP_PROC_NAME(void* param) noexcept(false); \
  }
#define BIFROST_PLUGIN_SETUP_PROC_DEF int BIFROST_PLUGIN_SETUP_PROC_NAME(void* param) noexcept(false)

#define BIFROST_PLUGIN_TEARDOWN_PROC_NAME bifrost_PluginTearDown
#define BIFROST_PLUGIN_TEARDOWN_PROC_NAME_STRING "bifrost_PluginTearDown"
#define BIFROST_PLUGIN_TEARDOWN_PROC_TYPE int (*)(void*) noexcept(false)
#define BIFROST_PLUGIN_TEARDOWN_PROC_DECL                                                   \
  extern "C" {                                                                              \
  __declspec(dllexport) int BIFROST_PLUGIN_TEARDOWN_PROC_NAME(void* param) noexcept(false); \
  }
#define BIFROST_PLUGIN_TEARDOWN_PROC_DEF int BIFROST_PLUGIN_TEARDOWN_PROC_NAME(void* param) noexcept(false)

#define BIFROST_PLUGIN_MESSAGE_PROC_NAME bifrost_PluginHandleMessage
#define BIFROST_PLUGIN_MESSAGE_PROC_NAME_STRING "bifrost_PluginHandleMessage"
#define BIFROST_PLUGIN_MESSAGE_PROC_TYPE int (*)(const void*, int) noexcept(false)
#define BIFROST_PLUGIN_MESSAGE_PROC_DECL                                                                         \
  extern "C" {                                                                                                   \
  __declspec(dllexport) int BIFROST_PLUGIN_MESSAGE_PROC_NAME(const void* data, int sizeInBytes) noexcept(false); \
  }
#define BIFROST_PLUGIN_MESSAGE_PROC_DEF int BIFROST_PLUGIN_MESSAGE_PROC_NAME(const void* data, int sizeInBytes) noexcept(false)

#define BIFROST_PLUGIN_HELP_PROC_NAME bifrost_PluginHelp
#define BIFROST_PLUGIN_HELP_PROC_NAME_STRING "bifrost_PluginHelp"
#define BIFROST_PLUGIN_HELP_PROC_TYPE const char* (*)() noexcept(false)
#define BIFROST_PLUGIN_HELP_PROC_DECL                                                \
  extern "C" {                                                                       \
  __declspec(dllexport) const char* BIFROST_PLUGIN_HELP_PROC_NAME() noexcept(false); \
  }
#define BIFROST_PLUGIN_HELP_PROC_DEF const char* BIFROST_PLUGIN_HELP_PROC_NAME() noexcept(false)

#pragma endregion
