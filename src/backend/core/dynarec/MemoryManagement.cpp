#include <core/Dynarec.hpp>

namespace n64::JIT {
void Dynarec::InvalidatePage(u32 paddr) {
  blockCache[paddr >> 20] = nullptr;
}

void Dynarec::InvalidateCache() {
  sizeUsed = 0;
  for(int i = 0; i < 0x80000; i++) {
    blockCache[i] = nullptr;
  }
}

void* Dynarec::bumpAlloc(u64 size, u8 val) {
  if(sizeUsed + size >= CODECACHE_SIZE) {
    InvalidateCache();
  }

  void* ptr = &codeCache[sizeUsed];
  sizeUsed += size;

  memset(ptr, val, size);

  return ptr;
}
}