//   ____  _  __               _
//  |  _ \(_)/ _|             | |
//  | |_) |_| |_ _ __ ___  ___| |_
//  |  _ <| |  _| '__/ _ \/ __| __|
//  | |_) | | | | | | (_) \__ \ |_
//  |____/|_|_| |_|  \___/|___/\__|   2018 - 2019
//
//
// This file is distributed under the MIT License (MIT).
// See LICENSE.txt for details.

#include "bifrost/core/common.h"

#include "bifrost/core/hook_settings.h"
#include "bifrost/core/context.h"
#include "bifrost/core/ilogger.h"
#include "bifrost/core/exception.h"
#include "bifrost/core/type.h"
#include "bifrost/core/error.h"
#include "bifrost/core/json.h"

namespace bifrost {

namespace {

template <class T, class JsonExtractFuncT, class EnvExtractFuncT, class LogExtractT>
T TryParseImpl(Context* ctx, const char* name, const Json& j, JsonExtractFuncT&& jsonExtractFunc, const char* envVar, EnvExtractFuncT&& envExtractFunc,
               LogExtractT&& logExtract, T defaultValue) {
  T value = defaultValue;

  // Try to extract the value from json
  if (j.count(name)) {
    try {
      value = jsonExtractFunc(j[name]);
    } catch (std::exception& e) {
      ctx->Logger().WarnFormat("Failed to extract hook setting \"%s\" from Json: %s", name, e.what());
    }
  }

  // Try to extract the value from env variable
  if (const char* env; env = std::getenv(envVar)) {
    try {
      value = envExtractFunc(env);
    } catch (std::exception& e) {
      ctx->Logger().WarnFormat("Failed to extract hook setting \"%s\" from env variable %s: %s", name, envVar, e.what());
    }
  }

  ctx->Logger().DebugFormat("  %-20s: %s", name, logExtract(value));
  return value;
}

static bool TryParseAsBool(Context* ctx, const char* name, const Json& j, const char* envVar, bool defaultValue) {
  return TryParseImpl(
      ctx, name, j, [](const Json& value) { return value.get<bool>(); }, envVar,
      [&defaultValue](const char* value) -> bool {
        std::string_view view = value;
        if (view == "1" || StringCompareCaseInsensitive(view, "true")) return true;
        if (view == "0" || StringCompareCaseInsensitive(view, "false")) return false;
        throw Exception("Failed to parse string \"%s\" as boolean", value);
        return defaultValue;
      },
      [](bool value) { return value ? "true" : "false"; }, defaultValue);
}

static bool TryParseAsInt(Context* ctx, const char* name, const Json& j, const char* envVar, i32 defaultValue) {
  return TryParseImpl(
      ctx, name, j, [](const Json& value) { return value.get<i32>(); }, envVar,
      [&defaultValue](const char* value) {
        i32 v = defaultValue;
        std::istringstream ss(value);
        ss >> v;
        if (ss.fail()) throw Exception("Failed to parse string \"%s\" as integer", value);
        return v;
      },
      [](i32 value) { return std::to_string(value); }, defaultValue);
}

template <class EnumT, class EnumExtractorT, class EnumPrinterT>
EnumT TryParseAsEnum(Context* ctx, const char* name, const Json& j, const char* envVar, EnumT defaultValue, EnumExtractorT&& enumExtractor,
                     EnumPrinterT&& enumPrinter) {
  return TryParseImpl(
      ctx, name, j, [&](const Json& value) { return enumExtractor(value.get<std::string>().c_str()); }, envVar,
      [&](const char* value) { return enumExtractor(value); }, [&](EnumT value) { return enumPrinter(value); }, defaultValue);
}

}  // namespace

HookSettings::HookSettings(Context* ctx) {
  // Try to read from hooks.json
  std::string hookFile = (std::filesystem::current_path() / L"hook.json").string();
  if (const char* var; var = std::getenv("BIFROST_HOOK_FILE")) hookFile = var;

  Json j;
  if (std::filesystem::exists(hookFile)) {
    std::ifstream ifs(hookFile);
    try {
      j = Json::parse(ifs);
    } catch (std::exception& e) {
      ctx->Logger().WarnFormat("Failed to read hook settings file \"%s\": %s", hookFile.c_str(), e.what());
    }
  }

  ctx->Logger().Debug("Hook settings:");

  // Debug
  Debug = TryParseAsBool(ctx, "Debug", j, "BIFROST_HOOK_DEBUG", Debug);

  // VerboseDbgHelp
  VerboseDbgHelp = TryParseAsBool(ctx, "VerboseDbgHelp", j, "BIFROST_HOOK_VERBOSE_DBGHELP", VerboseDbgHelp);

  // Strategy
  HookStrategy = TryParseAsEnum(
      ctx, "Strategy", j, "BIFROST_HOOK_STRATEGY", HookStrategy,
      [this](const char* str) -> EHookStrategy {
        if (StringCompareCaseInsensitive(str, "multi")) return EHookStrategy::E_Multi;
        if (StringCompareCaseInsensitive(str, "single")) return EHookStrategy::E_Single;
        throw Exception("Failed to parse string \"%s\" as HookStrategy enum", str);
      },
      [](EHookStrategy strategy) { return ToString(strategy); });
}

const char* ToString(EHookStrategy type) {
  switch (type) {
    case EHookStrategy::E_Multi:
      return "multi";
    case EHookStrategy::E_Single:
      return "single";
  };
  return "unknown";
}

}  // namespace bifrost