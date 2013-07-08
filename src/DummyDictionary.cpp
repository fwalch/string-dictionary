#include "DummyDictionary.hpp"

// Seems to work with clang, too
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
bool DummyDictionary::bulkInsert(size_t size, std::string* values) {
  nextId += size;
  return true;
}

uint64_t DummyDictionary::insert(std::string value) {
  nextId++;
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
