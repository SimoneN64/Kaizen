#include <core/JIT.hpp>

namespace n64 {
void JIT::InvalidatePage(u32 paddr) {
  blockCache[paddr >> 20] = nullptr;
}

void JIT::InvalidateCache() {
  sizeUsed = 0;
  for(auto &i : blockCache) {
    i = nullptr;
  }
}

void* JIT::bumpAlloc(u64 size, u8 val) {
  if(sizeUsed + size >= CODECACHE_SIZE) {
    InvalidateCache();
  }

  void* ptr = &codeCache[sizeUsed];
  sizeUsed += size;

  memset(ptr, val, size);

  return ptr;
}
}