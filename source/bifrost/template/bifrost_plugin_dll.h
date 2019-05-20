struct bfp_Plugin;

namespace bifrost {

namespace {

/// Get the most recent Win32 error
std::string GetLastWin32Error() {
  DWORD errorCode = ::GetLastError();
  if (errorCode == 0) return "Unknown Error.\n";  // No error message has been recorded

  LPSTR messageBuffer = nullptr;
  size_t size = ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, errorCode,
                                 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

  std::string message(messageBuffer, size);
  ::LocalFree(messageBuffer);
  return message;
}

/// Interaction with bifrost_plugin.dll
class BifrostPluginApi {
 private:
  using bfp_PluginInit_fn = bfp_Plugin* (*)(void);
  bfp_PluginInit_fn bfp_PluginInit;

  BifrostPluginApi() {
    HMODULE hModule = NULL;
    Check((hModule = ::LoadLibraryW(L"bifrost_plugin.dll")) != NULL);
    Check((bfp_PluginInit = (bfp_PluginInit_fn)::GetProcAddress(hModule, "bfp_PluginInit")) != NULL);
  }

 private:
  void Check(bool success) {
    if (!success) {
      // GetLastError()
    }
  }
};
static BifrostPluginApi* api = nullptr;
static std::mutex mutex;

/// Free the API singleton (thread-safe)
static void FreeApi() {
  std::lock_guard<std::mutex> lock(mutex);
  if (api) {
    delete api;
    api = nullptr;
  }
}

/// Access the API singleton (thread-safe)
static BifrostPluginApi& GetApi() {
  if (!api) {
    std::lock_guard<std::mutex> lock(mutex);
    if (!api) {
      api = new BifrostPluginApi();
      std::atexit(FreeApi);
    }
  }
  return *api;
}

#pragma endregion

}  // namespace

}  // namespace bifrost