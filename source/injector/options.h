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

#pragma once

#include "injector/common.h"
#include "bifrost/core/exception.h"

namespace injector {

/// Option interface
class IOption {
 public:
  virtual void GetValue(std::any& value) = 0;
  virtual args::FlagBase* GetFlag() = 0;
};

/// Option implementation
template <class FlagT, class ValueT>
class Option final : public IOption {
 public:
  Option(std::unique_ptr<FlagT>&& flag, ValueT&& defaultValue) : m_flag(std::move(flag)), m_defaultValue(std::forward<ValueT>(defaultValue)) {}

  virtual void GetValue(std::any& value) override {
    if (m_flag->Matched()) {
      value = m_flag->Get();
    } else {
      value = m_defaultValue;
    }
  }

  virtual args::FlagBase* GetFlag() override { return m_flag.get(); }

 private:
  std::unique_ptr<FlagT> m_flag;
  ValueT m_defaultValue;
};

/// Collection of options
class OptionCollection {
 public:
  OptionCollection(args::Subparser& parser) : m_parser(&parser) {}

  template <class ValueT, class FlagT>
  void AddOption(u32 id, ValueT value, FlagT* flag) {
    m_options.emplace(id, std::make_unique<Option<FlagT, ValueT>>(std::unique_ptr<FlagT>(flag), std::move(value)));
  }

  template <class T>
  T GetValue(u32 id) {
    std::any result;
    GetOption(id).GetValue(result);

    try {
      return std::any_cast<T>(result);
    } catch (std::bad_any_cast& e) {
      throw std::runtime_error(StringFormat("Cannot convert type \"%s\" to \"%s\": %s", result.type().name(), typeid(T).name(), e.what()));
    }
    return T();
  }

  args::FlagBase* GetFlag(u32 id) { return GetOption(id).GetFlag(); }

 private:
  IOption& GetOption(u32 id) {
    auto it = m_options.find(id);
    if (it == m_options.end()) throw bifrost::Exception("Internal error: Option %u does not exist", id);
    return *it->second;
  }

 private:
  args::Subparser* m_parser;
  std::unordered_map<u32, std::unique_ptr<IOption>> m_options;
};

}  // namespace injector
