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

  /// Virtual destructor
  virtual ~Plugin();

  /// Access the singleton instance as `T`
  template <class T>
  static T& GetAs() {
    return *(T*)&Get();
  }

	/// Hook description
  struct Hook {
    void* Orignal;  ///< Pointer to the original function/method
    void* Current;  ///< Pointer to the override of the original function/method
    bool Active;    ///< Is the hook currently active (meaning `Current` is called instead of `Original`)
  };

  //
  // INTERFACE
  //

  /// Called when setting up the plugin
  ///
  /// Use this function instead of the constructor.
  virtual void SetUp() = 0;

  /// Called when tearing down the plugin
  ///
  /// Use this function instead of the destructor.
  virtual void TearDown() = 0;

  /// Get the name of the plugin - by default returns the class name of the plugin i.e name passed to `BIFROST_REGISTER_PLUGIN`
  virtual const char* GetName() const;

  /// Handle incoming messages to this plugin - by default does nothing
  ///
  /// @param[in] data  Start of the message
  /// @param[in] size  Size of the message in bytes
  /// @return `true` if the message was handled successfully, `false` otherwise
  virtual bool HandleMessage(const void* data, int sizeInBytes);

  /// Called if a fatal exception occurred - by default logs to Error and throws std::runtime_error
  ///
  /// @param[in] msg   Error message which has to be '\0' terminated
  virtual void FatalError(const char* msg) const;
	
	//
	// HOOKING
	//
	//Hook* Hook(Identifer identifer, void* Override, bool activate = true);
  
	//
  // HELPER
  //

  /// Access the arguments which were passed to the plugin - calls Error on failure
  const char* GetArguments() const;

  /// Severity level
  enum class LogLevel : unsigned int { Debug = 0, Info, Warn, Error, Disable };

  /// Log a message - calls Error on failure
  ///
  /// @param[in] level         Severity level
  /// @param[in] msg           Message which has to be '\0' terminated
  /// @param[in] ignoreErrors  Don't call `FatalError` if something goes wrong
  void Log(LogLevel level, const char* msg, bool ignoreErrors = false) const;

	//
  // Internal use only
  //
  void _SetUpImpl(bfp_PluginContext_t*);
  void _TearDownImpl(bool);
  void _SetArguments(const char*);
  bfp_PluginContext_t* _GetPlugin();

 private:
  static Plugin* s_instance;
  static const char* s_name;

  bfp_PluginContext_t* m_plugin = nullptr;
  bool m_init = false;
  const char* m_arguments;
};

}  // namespace bifrost

/// Register a plugin
#define BIFROST_REGISTER_PLUGIN(plugin)         \
  ::bifrost::Plugin& ::bifrost::Plugin::Get() { \
    if (!s_instance) s_instance = new plugin;   \
    return *s_instance;                         \
  }                                             \
  const char* ::bifrost::Plugin::s_name = #plugin;

/// Register a help function which returns const char* and has no arguments (e.g `const char* Help()`)
#define BIFROST_REGISTER_PLUGIN_HELP(helpFunc) \
  BIFROST_PLUGIN_HELP_PROC_DECL                \
  BIFROST_PLUGIN_HELP_PROC_DEF { return helpFunc(); }

#pragma endregion

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
#define BIFROST_PLUGIN_MESSAGE_PROC_DECL                                                                          \
  extern "C" {                                                                                                    \
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
