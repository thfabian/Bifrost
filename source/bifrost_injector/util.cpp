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

#include "bifrost_injector/common.h"
#include "bifrost_injector/util.h"
#include "bifrost_core/error.h"

namespace bifrost::injector {

TerminalInfo GetTerminalInfo() {
  CONSOLE_SCREEN_BUFFER_INFO csbi;
  BIFROST_CHECK_WIN_CALL(GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi) != FALSE);
  int columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
  int rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
  return {columns, rows};
}

}  // namespace bifrost::injector
