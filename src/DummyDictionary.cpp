#include "DummyDictionary.hpp"

// Seems to work with clang, too
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
uint64_t DummyDictionary::insert(std::string value) {
  return 0;
}

bool DummyDictionary::update(uint64_t& id, std::string value) {
  return true;
}

bool DummyDictionary::lookup(std::string value, uint64_t& id) {
  return true;
}

bool DummyDictionary::lookup(uint64_t id, std::string& value) {
  return true;
}

#pragma GCC diagnostic pop
