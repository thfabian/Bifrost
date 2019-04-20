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
#include "bifrost/core/sm_context.h"

namespace bifrost {

SMContext* SMContext::Create(SharedMemory* mem, u64 memorySize) {
  // Allocate memory (this is never released)
  void* firstAddress = mem->Allocate(sizeof(SMContext));
  if (!firstAddress) {
    throw std::runtime_error("Failed to allocate memory for SMContext");
  }
  BIFROST_ASSERT(firstAddress == mem->GetFirstAdress() && "First address allocated does not match GetFirstAdress()");

  auto smCtx = (SMContext*)firstAddress;
  ::new (smCtx) SMContext();

  BIFROST_LOCK_GUARD(smCtx->m_mutex);
  smCtx->m_refCount = 1;
  smCtx->m_memorySize = memorySize;
  smCtx->m_storage = New<SMStorage>(mem);
  smCtx->m_logstash = New<SMLogStash>(mem);
  return smCtx;
}

SMContext* SMContext::Map(void* firstAdress) {
  auto smCtx = (SMContext*)firstAdress;
  BIFROST_LOCK_GUARD(smCtx->m_mutex);
  BIFROST_ASSERT(smCtx->m_refCount > 0);
  smCtx->m_refCount++;
  return smCtx;
}

void SMContext::Destruct(SharedMemory* mem, SMContext* smCtx) {
  BIFROST_LOCK_GUARD(smCtx->m_mutex);
  if (--smCtx->m_refCount == 0) {
    Delete(mem, smCtx->m_storage);
    Delete(mem, smCtx->m_logstash);
  }
}

}  // namespace bifrost