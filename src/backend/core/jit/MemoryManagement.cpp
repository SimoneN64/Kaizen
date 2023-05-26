#include <core/JIT.hpp>

namespace n64 {
void JIT::InvalidatePage(u32 paddr) {
  Util::aligned_free(blockCache[paddr >> 20]);
}

void JIT::InvalidateCache() {
  sizeUsed = 0;
  for(auto &i : blockCache) {
    Util::aligned_free(i);
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