#include "HashARTDictionary.hpp"
#include "boost/algorithm/string.hpp"

using namespace std;

uint64_t HashARTDictionary::insert(string value) {
  uint64_t id;
  if (!reverseIndex.lookup(value, id)) {
    // String not in dictionary
    reverseIndex.insert(value, nextId);
    index.insert(make_pair(nextId, value));
    return nextId++;
  }
  return id;
}

bool HashARTDictionary::bulkInsert(size_t size, string* values) {
  assert(nextId == 0);

  for (; nextId < size; nextId++) {
    reverseIndex.insert(values[nextId], nextId);
    index.insert(make_pair(nextId, values[nextId]));
  }

  return true;
}

bool HashARTDictionary::update(uint64_t& id, std::string value) {
  auto it = index.find(id);
  if (it == index.end()) {
    return false;
  }

  if (value != it->second) {
    id = insert(value);
  }
  return true;
}

bool HashARTDictionary::lookup(std::string value, uint64_t& id) {
  return reverseIndex.lookup(value, id);
}

bool HashARTDictionary::lookup(uint64_t id, std::string& value) {
  auto it = index.find(id);
  if (it == index.end()) {
    return false;
  }

  value = it->second;
  return true;
}

Dictionary::Iterator HashARTDictionary::rangeLookup(std::string prefix) {
  return Iterator(this, prefix);
}

HashARTDictionary::Iterator::Iterator(HashARTDictionary* dictionary, std::string pref) : dict(dictionary), prefix(pref), iterator(dictionary->index.cbegin()) {
}

Dictionary::Iterator& HashARTDictionary::Iterator::operator++() {
  ++iterator;
  return *this;
}

HashARTDictionary::Iterator::operator bool() {
  return iterator != dict->index.cend()
    && boost::starts_with(iterator->second, prefix);
}

const std::pair<uint64_t, std::string> HashARTDictionary::Iterator::operator*() {
  return std::make_pair(iterator->first, iterator->second);
}
