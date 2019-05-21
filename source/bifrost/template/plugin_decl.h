//#include <vector>
//#include <functional>

//#pragma region Macros

//#define BIFROST_INITIALIZER_IMPL(f, p)                     \
//  __pragma(section(".CRT$XCU", read)) static void f(void); \
//  __declspec(allocate(".CRT$XCU")) void (*f##_)(void) = f;
//
///// The wrapped function will be executed at module load
//#ifdef _WIN64
//#define BIFROST_INITIALIZER(f) BIFROST_INITIALIZER_IMPL(f, "")
//#else
//#define BIFROST_INITIALIZER(f) BIFROST_INITIALIZER_IMPL(f, "_")
//#endif

//#ifndef BIFROST_STRINGIFY_IMPL
//#define BIFROST_STRINGIFY_IMPL(x) #x
//#endif
//
///// Convert `x` to a string constant
//#ifndef BIFROST_STRINGIFY
//#define BIFROST_STRINGIFY(x) BIFROST_STRINGIFY_IMPL(x)
//#endif
//
///// Concatenate `a` an `b`
//#ifndef BIFROST_CONCAT
//#define BIFROST_CONCAT(a, b) a##b
//#endif

//#pragma endregion

#pragma region Plugin Procedure

#define BIFROST_PLUGIN_SETUP_PROC_NAME bifrost_PluginSetUp
#define BIFROST_PLUGIN_SETUP_PROC_NAME_STRING "bifrost_PluginSetUp"
#define BIFROST_PLUGIN_SETUP_PROC_TYPE BOOL (*)(LPVOID) noexcept(false)
#define BIFROST_PLUGIN_SETUP_PROC_DECL                                                     \
  extern "C" {                                                                             \
  __declspec(dllexport) BOOL BIFROST_PLUGIN_SETUP_PROC_NAME(LPVOID param) noexcept(false); \
  }
#define BIFROST_PLUGIN_SETUP_PROC_DEF BOOL BIFROST_PLUGIN_SETUP_PROC_NAME(LPVOID param) noexcept(false)

#pragma endregion

#pragma region Plugin Interface

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

  /// Internal use only
  void _SetUpImpl() {}

  /// Internal use only
  void _TearDownImpl() {}

  /// Internal use only
  void _SetPlugin(bfp_Plugin* plugin) { m_plugin = plugin; }

 private:
  static Plugin* s_instance;
  bfp_Plugin* m_plugin;
};

// namespace internal {
//
// class PluginManager {
// public:
//  static PluginManager& Get() {
//    if (!m_instance) {
//      m_instance = new PluginManager;
//    }
//    return *m_instance;
//  }
//
//  void AddConstructor(std::function<Plugin*(void)>&& constructor) { m_constructors.emplace_back(std::move(constructor)); }
//  const std::vector<std::function<Plugin*(void)>>& GetConstructors() const { return m_constructors; }
//
// private:
//  static PluginManager* m_instance;
//  std::vector<std::function<Plugin*(void)>> m_constructors;
//};
//
//}  // namespace internal
//

/// Register a plugin
#define BIFROST_REGISTER_PLUGIN(plugin)         \
  ::bifrost::Plugin& ::bifrost::Plugin::Get() { \
    if (!s_instance) s_instance = new plugin;   \
    return *s_instance;                         \
  }

//#define BIFROST_REGISTER_PLUGIN(plugin)                                                                          \
//  BIFROST_INITIALIZER(BIFROST_CONCAT(plugin, __Registration)) {                                                  \
//    ::bifrost::internal::PluginManager::Get().AddConstructor([]() -> ::bifrost::Plugin* { return new plugin; }); \
//  }

}  // namespace bifrost

#pragma endregion
