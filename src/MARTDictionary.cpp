#include "MARTDictionary.hpp"
#include <cassert>
#include <tuple>
#include "Page.hpp"

using namespace std;

uint64_t MARTDictionary::insert(string value) {
  uint64_t id;
  if (!reverseIndex.lookup(value, id)) {
    // String not in dictionary
    reverseIndex.insert(value, nextId);
    index.insert(nextId, value);
    return nextId++;
  }
  return id;
}

bool MARTDictionary::bulkInsert(size_t size, string* values) {
  assert(nextId == 0);

  vector<pair<uint64_t, string>> insertValues;
  insertValues.reserve(size);

  for (size_t i = 0; i < size; i++) {
    insertValues.push_back(make_pair(nextId++, values[i]));
  }

  typedef Page<16*1024, 16> pageType;
  vector<pageType*> pages;
  pageType* page = nullptr;
  const char* endOfPage = nullptr;
  char* dataPtr = nullptr;
  const uint64_t headerSize = sizeof(uint8_t) + 2*sizeof(uint64_t);

  for (const auto& pair : insertValues) {
    if (dataPtr != nullptr && dataPtr + headerSize + pair.second.size() > endOfPage) {
      pages.push_back(page);
      page = nullptr;
    }

    if (page == nullptr) {
      page = new pageType();
      dataPtr = page->data;
      endOfPage = page->data + page->size;
    }
    page->startPrefix(dataPtr);
    page->writeId(dataPtr, pair.first);
    page->writeValue(dataPtr, pair.second);
  }

  reverseIndex.bulkInsert(size, values);

  for (; nextId < size; nextId++) {
    index.insert(nextId, values[nextId]);
  }

  return true;
}

bool MARTDictionary::update(uint64_t& id, std::string value) {
  std::string storedValue;
  if (!index.lookup(id, storedValue)) {
    return false;
  }

  if (value != storedValue) {
    id = insert(value);
  }
  return true;
}

bool MARTDictionary::lookup(std::string value, uint64_t& id) {
  return reverseIndex.lookup(value, id);
}

bool MARTDictionary::lookup(uint64_t id, std::string& value) {
  return index.lookup(id, value);
}
