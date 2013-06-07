#include "IndexART.hpp"
#include <unordered_map>
#include "ReverseIndexART.hpp"

//TODO: get rid of memcpys
template<>
void ReverseIndexART<IndexART>::loadKey(uintptr_t tid, uint8_t keyArray[]) {
  std::string key = index.values.at(tid);
  memcpy(keyArray, key.c_str(), key.size() + 1);
}

template<>
void ReverseIndexART<std::unordered_map<uint64_t, std::string>>::loadKey(uintptr_t tid, uint8_t keyArray[]) {
  std::string key = index.at(tid);
  memcpy(keyArray, key.c_str(), key.size() + 1);
}
