#include "HARTDictionary.hpp"
#include <vector>

using namespace std;

uint8_t* ART::loadKey(uintptr_t leafValue) {
  uint64_t tid = (*dict->decodeLeaf(leafValue)).id;

  reinterpret_cast<uint64_t*>(lastKey)[0] = __builtin_bswap64(tid);
  return lastKey;
}

bool ART::lookup(uint64_t key, uint64_t& value) {
  uint8_t swappedKey[8];
  reinterpret_cast<uint64_t*>(swappedKey)[0] = __builtin_bswap64(key);

  Node* leaf = lookupValue(tree, swappedKey, sizeof(uint64_t), 0);
  if (leaf == nullNode) {
    return false;
  }
  assert(isLeaf(leaf));
  value = getLeafValue(leaf);
  return true;
}

HARTDictionary::HARTDictionary() : index(this) {
  reverseIndex = hattrie_create();
}

HARTDictionary::~HARTDictionary() noexcept {
  hattrie_free(reverseIndex);
}


uint64_t HARTDictionary::insert(string value) {
  bool inserted = false;
  uint64_t* idPtr = hattrie_get(reverseIndex, value.c_str(), value.size() + 1, &inserted);
  if (inserted) {
    // Create leaf

    //TODO
    /**idPtr = addressOfOurLeaf;
    index.insert(nextId, addressOfOurLeaf);*/
    return nextId++;
  }

  return *idPtr;
}

uint64_t HARTDictionary::encodeLeaf(PageType* page, uint16_t deltaNumber) {
  uint64_t leafValue = reinterpret_cast<uint64_t>(page);
  leafValue = leafValue << 16;
  leafValue |= static_cast<uint64_t>(deltaNumber);

  return leafValue;
}

void HARTDictionary::insertLeaf(PageType* page, uint16_t deltaNumber, page::IdType id, std::string value) {
  uint64_t leafValue = encodeLeaf(page, deltaNumber);

  bool inserted = false;
  uint64_t* valuePtr = hattrie_get(reverseIndex, value.c_str(), value.size()+1, &inserted);
  *valuePtr = leafValue;
  assert(inserted);

  index.insert(id, leafValue);
}

HARTDictionary::PageType::Iterator HARTDictionary::decodeLeaf(uint64_t leafValue) {
  PageType* page = reinterpret_cast<PageType*>(leafValue >> 16);
  uint16_t deltaNumber = leafValue & 0xFFFF;

  return page->get(deltaNumber);
}

bool HARTDictionary::bulkInsert(size_t size, string* values) {
  assert(nextId == 0);

  vector<pair<uint64_t, string>> insertValues;
  insertValues.reserve(size);

  for (size_t i = 0; i < size; i++) {
    insertValues.push_back(make_pair(nextId++, values[i]));
  }

  PageType::Loader loader;
  PageType::Loader::CallbackType callback;
  callback = std::bind(&HARTDictionary::insertLeaf, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
  loader.load(insertValues, callback);

  return true;
}

bool HARTDictionary::update(uint64_t& id, std::string value) {
  uint64_t leafPtr;
  if (!index.lookup(id, leafPtr)) {
    return false;
  }

  PageType* page = *reinterpret_cast<PageType**>(leafPtr);
  auto iterator = page->getId(id);
  assert(iterator);

  if (value != (*iterator).value) {
    id = insert(value);
  }
  return true;
}

bool HARTDictionary::lookup(std::string value, uint64_t& id) {
  uint64_t* leafPtr = hattrie_tryget(reverseIndex, value.c_str(), value.size() + 1);
  if (leafPtr == NULL) {
    return false;
  }

  auto iterator = decodeLeaf(*leafPtr);
  assert(iterator);

  id = (*iterator).id;
  return true;
}

bool HARTDictionary::lookup(uint64_t id, std::string& value) {
  uint64_t leafValue;
  if (index.lookup(id, leafValue)) {
    auto iterator = decodeLeaf(leafValue);
    assert(iterator);

    value = (*iterator).value;
    return true;
  }
  return false;
}
