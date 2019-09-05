// The following methods are also used by bifrost_plugin.dll and are thus implemented here to be shareable.
// (!) Changing the signature of these function requires bumping the major version in bifrost/api/plugin.h, changing the implementation significantly requires
// changing the minor version.

BIFROST_NAMESPACE_BEGIN

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
  //DeactivateAllHooks();
  m_init = false;
}

void Plugin::_SetArguments(const char* arguments) {
  std::string str(arguments);

  try {
    m_arguments = new char[str.size() + 1];
    std::memcpy((void*)m_arguments, str.c_str(), str.size() + 1);
  } catch (...) {
    m_arguments = nullptr;
  }
}

BIFROST_NAMESPACE_END
