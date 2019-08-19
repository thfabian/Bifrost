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

#include "injector/common.h"
#include "injector/logger.h"
#include "bifrost/core/macros.h"
#include "bifrost/core/mutex.h"
#include "bifrost/core/ilogger.h"

namespace injector {

std::unique_ptr<Logger> Logger::m_instance = nullptr;

Logger& Logger::Get() {
  if (!m_instance) {
    m_instance = std::make_unique<Logger>();
  }
  return *m_instance;
}

std::shared_ptr<Logger::StderrSinkT> Logger::MakeStderrSink() {
  auto sink = std::make_shared<StderrSinkT>();
  sink->set_color(spdlog::level::debug, sink->WHITE);
  sink->set_color(spdlog::level::info, sink->WHITE | sink->BOLD);
  sink->set_color(spdlog::level::warn, sink->YELLOW | sink->BOLD);
  sink->set_color(spdlog::level::err, sink->RED | sink->BOLD);
  return sink;
}

std::shared_ptr<Logger::FileSinkT> Logger::MakeFileSink(const std::filesystem::path& path) { return std::make_shared<FileSinkT>(path.string()); }

std::shared_ptr<Logger::MsvcSinkT> Logger::MakeMsvcSink() { return std::make_shared<MsvcSinkT>(); }

void Logger::Log(u32 level, const char* module, const char* msg) {
  if(!m_logger) return;

  auto logLevel = (ILogger::LogLevel)level;
  auto spdLevel = spdlog::level::off;

  switch (logLevel) {
    case ILogger::LogLevel::Debug:
      spdLevel = spdlog::level::debug;
      break;
    case ILogger::LogLevel::Info:
      spdLevel = spdlog::level::info;
      break;
    case ILogger::LogLevel::Warn:
      spdLevel = spdlog::level::warn;
      break;
    case ILogger::LogLevel::Error:
      spdLevel = spdlog::level::err;
      break;
    case ILogger::LogLevel::Disable:
    default:
      spdLevel = spdlog::level::off;
      break;
  }

  BIFROST_LOCK_GUARD(m_mutex);
  m_buffer.clear();
  if (!std::string_view(module).empty()) {
    m_buffer += "[";
    m_buffer += module;
    m_buffer += "] ";
  }
  m_buffer += msg;
  GetLogger()->log(spdLevel, m_buffer.c_str());
}

Logger::Logger() {}

Logger::~Logger() {
  m_sinks.clear();
  m_logger.reset();
}

void Logger::AddSink(const std::string& name, const std::shared_ptr<LogSinkT>& sink) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_sinks[name] = sink;
  MakeLogger();
}

void Logger::AddSinks(const std::unordered_map<std::string, std::shared_ptr<LogSinkT>>& sinks) {
  BIFROST_LOCK_GUARD(m_mutex);
  for (auto sink : sinks) m_sinks[sink.first] = sink.second;
  MakeLogger();
}

void Logger::RemoveSink(const std::string& name) {
  BIFROST_LOCK_GUARD(m_mutex);
  m_sinks.erase(name);
  MakeLogger();
}

void Logger::SetModule(const char* module) { m_module = module; }
void Logger::Sink(LogLevel level, const char* module, const char* msg) { Log((u32)level, module, msg); }
void Logger::Sink(LogLevel level, const char* msg) { Sink(level, m_module.c_str(), msg); }

void Logger::MakeLogger() {
  if (m_logger) spdlog::drop("injector");

  std::vector<spdlog::sink_ptr> sinkVec;
  for (const auto& sink : m_sinks) sinkVec.emplace_back(sink.second);

  try {
    m_logger = std::make_shared<spdlog::logger>("injector", sinkVec.begin(), sinkVec.end());
    m_logger->set_level(spdlog::level::trace);
    m_logger->set_pattern("[%H:%M:%S.%e] [%-5l] %v");
    spdlog::register_logger(m_logger);
  } catch (const spdlog::spdlog_ex& ex) {
    throw std::runtime_error(fmt::format("Cannot create logger: {}", ex.what()));
  }
}

void LogCallback(u32 level, const char* module, const char* msg) { Logger::Get().Log(level, module, msg); }

}  // namespace injector
