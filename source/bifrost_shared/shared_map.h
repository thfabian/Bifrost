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

#include "bifrost_shared/bifrost_shared.h"
#include "bifrost_shared/stl.h"
#include "bifrost_core/non_copyable.h"

namespace bifrost::shared {

/// Shared key/value storage - thread safe
class BIFROST_SHARED_API SharedMap : NonCopyable {
 public:
  using list_t = stl::list<stl::string>;

  struct Value {
    bfs_Value* Data;
    list_t::iterator PathIterator;
  };
  using map_t = stl::unordered_map<std::string_view, Value>;

  SharedMap();
  ~SharedMap();

  /// Read the value ``value`` from ``path``
  bfs_Status Read(std::string_view path, bfs_Value* value, bool deepCopy);

  /// Write ``value`` to ``path``
  bfs_Status Write(std::string_view path, const bfs_Value* value);

  /// Free allocated value
  void FreeValue(bfs_Value* value);

  /// Get a list of all paths
  bfs_Status GetPaths(bfs_PathList* paths);

  /// Free the list of paths
  void FreePaths(bfs_PathList* paths);

 private:
  std::mutex m_mutex;
  list_t m_paths;
  map_t m_mapping;
};

}  // namespace bifrost::shared