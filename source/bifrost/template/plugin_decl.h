#pragma region Plugin Interface

struct bfp_PluginContext_t;

namespace bifrost {

/// Plugin interface to be implemented
///
/// Register your plugin via the macro `BIFROST_REGISTER_PLUGIN`.
class Plugin {
 public:
  /// Access the singleton instance
  static Plugin& Get();

  /// Access the singleton instance as `T`
  template <class T>
  static T& GetAs() {
    return *(T*)&Get();
  }

  /// Called when setting up the plugin
  ///
  /// Use this function instead of the constructor.
  virtual void SetUp() = 0;

  /// Called when tearing down the plugin
  ///
  /// Use this function instead of the destructor.
  virtual void TearDown() = 0;

  /// Get the name of the plugin
  virtual const char* GetName() { return s_name; }

  /// Severity level
  enum class LogLevel : unsigned int { Debug = 0, Info, Warn, Error, Disable };

  /// Log a message - throws on error
  ///
  /// @param[in] level  Severity level
  /// @param[in] msg    Message which has to be '\0' terminated
  void Log(LogLevel level, const char* msg);

  /// Internal use only
  void _SetUpImpl(bfp_PluginContext_t* plugin);

  /// Internal use only
  void _TearDownImpl(bool noFail);

  /// Internal use only
  bfp_PluginContext_t* _GetPlugin();

 private:
  static Plugin* s_instance;
  static const char* s_name;
  bfp_PluginContext_t* m_plugin = nullptr;
  bool m_init = false;
};

}  // namespace bifrost

/// Register a plugin
#define BIFROST_REGISTER_PLUGIN(plugin)         \
  ::bifrost::Plugin& ::bifrost::Plugin::Get() { \
    if (!s_instance) s_instance = new plugin;   \
    return *s_instance;                         \
  }                                             \
  const char* ::bifrost::Plugin::s_name = #plugin;

#pragma endregion

#pragma region Implementation
#if defined(BIFROST_IMPLEMENTATION) || defined(__INTELLISENSE__)

#include <stdexcept>

namespace bifrost {

void Plugin::_SetUpImpl(bfp_PluginContext_t* plugin) {
  m_plugin = plugin;
  if (m_init) throw std::runtime_error("Plugin already set up");
  SetUp();
  m_init = true;
}

void Plugin::_TearDownImpl(bool noFail) {
  if (noFail && !m_init) return;
  if (!m_init) throw std::runtime_error("Plugin not set up");
  TearDown();
  m_init = false;
}

}  // namespace bifrost

#endif
#pragma endregion

#pragma region Plugin Procedure

#define BIFROST_PLUGIN_SETUP_PROC_NAME bifrost_PluginSetUp
#define BIFROST_PLUGIN_SETUP_PROC_NAME_STRING "bifrost_PluginSetUp"
#define BIFROST_PLUGIN_SETUP_PROC_TYPE int32_t (*)(void*) noexcept(false)
#define BIFROST_PLUGIN_SETUP_PROC_DECL                                                       \
  extern "C" {                                                                               \
  __declspec(dllexport) int32_t BIFROST_PLUGIN_SETUP_PROC_NAME(void* param) noexcept(false); \
  }
#define BIFROST_PLUGIN_SETUP_PROC_DEF int32_t BIFROST_PLUGIN_SETUP_PROC_NAME(void* param) noexcept(false)

#define BIFROST_PLUGIN_TEARDOWN_PROC_NAME bifrost_PluginTearDown
#define BIFROST_PLUGIN_TEARDOWN_PROC_NAME_STRING "bifrost_PluginTearDown"
#define BIFROST_PLUGIN_TEARDOWN_PROC_TYPE int32_t (*)(void*) noexcept(false)
#define BIFROST_PLUGIN_TEARDOWN_PROC_DECL                                                       \
  extern "C" {                                                                                  \
  __declspec(dllexport) int32_t BIFROST_PLUGIN_TEARDOWN_PROC_NAME(void* param) noexcept(false); \
  }
#define BIFROST_PLUGIN_TEARDOWN_PROC_DEF int32_t BIFROST_PLUGIN_TEARDOWN_PROC_NAME(void* param) noexcept(false)

#pragma endregion
