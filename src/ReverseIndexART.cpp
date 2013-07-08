#include "IndexART.hpp"
#include <unordered_map>
#include "ReverseIndexART.hpp"

template<>
uint8_t* ReverseIndexART<IndexART>::loadKey(uintptr_t tid) {
  return cast(index.values[tid].second);
}

template<>
uint8_t* ReverseIndexART<std::unordered_map<uint64_t, std::string>>::loadKey(uintptr_t tid) {
  return cast(index[tid]);
}
