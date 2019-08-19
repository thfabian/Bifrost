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

#include "bifrost/core/test/test.h"
#include "bifrost/core/sm_log_stash.h"

namespace {

using namespace bifrost;

class SharedLogStashTest : public TestBaseSharedMemory {
 public:
  void Log(Context* ctx, ILogger::LogLevel level, const char* module, const char* msg) { ctx->Memory().GetSMLogStash()->Push(ctx, (u32)level, module, msg); }
};

class LogBuffer : public ILogger {
 public:
  struct Message {
    LogLevel Level;
    std::string Module;
    std::string Msg;
  };
  std::vector<Message> Buffer;

  virtual void SetModule(const char* module) override {}
  virtual void Sink(LogLevel level, const char* module, const char* msg) override { Buffer.emplace_back(Message{level, module, msg}); }
  virtual void Sink(LogLevel level, const char* msg) override { Buffer.emplace_back(Message{level, "", msg}); }
};

TEST_F(SharedLogStashTest, SingleSharedMemory) {
  // Log messages
  Log(GetContext(), ILogger::LogLevel::Debug, "module1", "msg1");

  // Consume them
  LogBuffer sink;
  LogStashConsumer consumer(GetContext(), GetContext()->Memory().GetSMLogStash(), &sink);
  ::Sleep(200);
  Log(GetContext(), ILogger::LogLevel::Warn, "module2", "msg2");
  Log(GetContext(), ILogger::LogLevel::Error, "module3", "msg3");

  consumer.StopAndFlush();

  // Check
  ASSERT_EQ(3, sink.Buffer.size());
  EXPECT_EQ(ILogger::LogLevel::Debug, sink.Buffer[0].Level);
  EXPECT_STREQ("module1", sink.Buffer[0].Module.c_str());
  EXPECT_STREQ("msg1", sink.Buffer[0].Msg.c_str());

  EXPECT_EQ(ILogger::LogLevel::Warn, sink.Buffer[1].Level);
  EXPECT_STREQ("module2", sink.Buffer[1].Module.c_str());
  EXPECT_STREQ("msg2", sink.Buffer[1].Msg.c_str());

  EXPECT_EQ(ILogger::LogLevel::Error, sink.Buffer[2].Level);
  EXPECT_STREQ("module3", sink.Buffer[2].Module.c_str());
  EXPECT_STREQ("msg3", sink.Buffer[2].Msg.c_str());
}

TEST_F(SharedLogStashTest, MultiSharedMemory) {
  Context& ctx1 = *GetContext();

  Context ctx2;
  ctx2.SetLogger(GetLogger());
  SharedMemory smem2(&ctx2, ctx1.Memory().GetName(), ctx1.Memory().GetSizeInBytes());
  ctx2.SetMemory(&smem2);

  Log(&ctx2, ILogger::LogLevel::Warn, "module1", "msg1");

  // Consume them
  LogBuffer sink;
  LogStashConsumer consumer(&ctx2, smem2.GetSMLogStash(), &sink);
  ::Sleep(200);
  Log(&ctx2, ILogger::LogLevel::Warn, "module2", "msg2");

  consumer.StopAndFlush();

  // Check
  ASSERT_EQ(2, sink.Buffer.size());
  EXPECT_STREQ("module1", sink.Buffer[0].Module.c_str());
  EXPECT_STREQ("msg1", sink.Buffer[0].Msg.c_str());
  EXPECT_STREQ("module2", sink.Buffer[1].Module.c_str());
  EXPECT_STREQ("msg2", sink.Buffer[1].Msg.c_str());
}

}  // namespace