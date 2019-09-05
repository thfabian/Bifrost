#pragma region Plugin Interface

struct bfp_PluginContext_t;

BIFROST_NAMESPACE_BEGIN

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

  //
  // IDENTIFIER
  //
  enum class Identifer : std::uint64_t {
    Unsused = 0,
#ifdef BIFROST_CODEGEN
    $BIFROST_PLUGIN_IDENTIFIER$
#elif defined(BIFROST_PLUGIN_TEST)
    saxpy,
#endif
        NumIdentifier
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

  /// Hook a plain "C" function
  class Hook {
   public:
    /// Get the function pointer to the original function/method
    inline void* Original() const noexcept { return m_original; }

    /// Get the function pointer to the override of the original function/method
    inline void* Override() const noexcept { return m_override; }

    /// Activates the hook, calls `FatalError` if an error occurred.
    void Activate();

    /// Tries to activate the hook, returns `true` on success.
    bool TryActivate();

    /// Deactivate the hook, calls `FatalError` if an error occurred.
    void Deactivate();

    /// Query if this hook is currently active
    inline bool IsActive() const noexcept { return m_isActive; }

    void _SetOverride(void* override) noexcept;
    void _SetOriginal(void* original) noexcept;

   private:
    void* m_original = nullptr;
    void* m_override = nullptr;
    bool m_isActive = false;
  };

  /// Hook the function given by `identifier` to call `override` instead
  ///
  /// If the hook has already been set, the hook is first deactivated and a new Hook object is created.
  /// Calls `FatalError` if an error occurred.
  ///
  /// @param[in] identifier Identifer of the function to override
  /// @param[in] override   Function pointer to use for the override
  /// @param[in] activate   Immediately activate the hook?
  /// @returns a reference to the hook object
  Hook* SetHook(Identifer identifier, void* override, bool activate = true);
  Hook* SetHook(const char* identifier, void* override, bool activate = true);

  /// Get an already set hook
  ///
  /// @param[in] identifier  Identifer of the hook
  /// @returns a reference to the hook object or NULL if the hook has not been set yet.
  Hook* GetHook(Identifer identifier) noexcept;
  Hook* GetHook(const char* identifier);

  /// Check if the `identifier` as a registered hook (the hook may be inactive)
  ///
  /// @param[in] identifier  Identifer of the hook
  /// @returns `true` if a hook has been set, false otherwise.
  bool HasHook(Identifer identifier) noexcept;
  bool HasHook(const char* identifier);

  /// Deactivate all hooks
  void DeactivateAllHooks() noexcept;

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
  // INTERNAL
  //

  template <Identifer identifer>
  inline constexpr Hook* _GetHook() noexcept {
    return &m_hooks[(std::uint64_t)identifer];
  }

  bfp_PluginContext_t* _GetPlugin();

  // (!) Changing the signature of these function requires bumping the major version in bifrost/api/plugin.h as bifrost/api/plugin_context.h relies on this
  // interface.
  void _SetUpImpl(bfp_PluginContext_t*);
  void _TearDownImpl(bool);
  void _SetArguments(const char*);

 private:
  static Plugin* s_instance;
  static const char* s_name;

  Hook m_hooks[(std::uint64_t)Identifer::NumIdentifier];

  bfp_PluginContext_t* m_plugin = nullptr;
  bool m_init = false;
  const char* m_arguments;
};

BIFROST_NAMESPACE_END

/// Define a plugin
#define BIFROST_PLUGIN BIFROST_NAMESPACE_UNQUALIFIED(Plugin)

/// Register a plugin
#define BIFROST_REGISTER_PLUGIN(plugin)       \
  BIFROST_NAMESPACE_BEGIN                     \
  Plugin& Plugin::Get() {                     \
    if (!s_instance) s_instance = new plugin; \
    return *s_instance;                       \
  }                                           \
  const char* Plugin::s_name = #plugin;       \
  BIFROST_NAMESPACE_END

/// Register a help function which returns const char* and has no arguments (e.g `const char* Help()`)
#define BIFROST_REGISTER_PLUGIN_HELP(helpFunc) \
  BIFROST_PLUGIN_HELP_PROC_DECL                \
  BIFROST_PLUGIN_HELP_PROC_DEF { return helpFunc(); }

#pragma endregion
