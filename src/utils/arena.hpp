#pragma once

struct ArenaBuffer {
  explicit ArenaBuffer(size_t maxSize = 0x1000) : maxSize(maxSize) {
    buffer = (char*)malloc(maxSize);
  }

  ~ArenaBuffer() {
    free(buffer);
  }

  template<typename T>
  void Write(const T& v) {
    memcpy(buffer + cursor, &v, sizeof(T));
    cursor += sizeof(T);
  }

  void Reset() { cursor = 0; }
  const void* GetBuffer() { return buffer; }
  [[nodiscard]] size_t GetSize() const { return cursor; }
private:
  char* buffer{};
  size_t maxSize=0;
  size_t cursor=0;
};

struct ArenaReadBuffer {
  ArenaReadBuffer(const char* buffer, size_t size) : buffer(buffer), size(size) {}

  template<typename T>
  T Read() {
    T* ret = (T*)(buffer + cursor);
    cursor += sizeof(T);
    return *ret;
  }
private:
  const char* buffer{};
  size_t size{};
  size_t cursor{};
};