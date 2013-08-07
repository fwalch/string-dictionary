#include "HARTDictionary.hpp"
#include <vector>

using namespace std;

uint8_t* ART::loadKey(uintptr_t leafValue) {
  uint64_t tid = (*dict->decodeLeafValue(leafValue)).id;

  reinterpret_cast<uint64_t*>(lastKey)[0] = __builtin_bswap64(tid);
  return lastKey;
}

bool ART::lookup(uint64_t key, uint64_t& value) {
  uint8_t swappedKey[8];
  reinterpret_cast<uint64_t*>(swappedKey)[0] = __builtin_bswap64(key);

  Node* leaf = lookupValue(tree, swappedKey, sizeof(uint64_t), 0);
  if (leaf == nullNode) {
    cout << "Leaf not found (ART) " << endl;
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

inline string delta(const string& ref, const string& value, uint64_t& pos) {
  pos = 0;
  while (pos < ref.size() && pos < value.size() && ref[pos] == value[pos]) {
    pos++;
  }

  auto delta = value.substr(pos, value.size()-pos);
  cout << "Delta between " << ref << " & " << value << ":" << delta << endl;
  return delta;
}

uint64_t HARTDictionary::encodeLeafValue(PageType* page, uint16_t deltaNumber) {
  uint64_t leafValue = reinterpret_cast<uint64_t>(page);
  leafValue = leafValue << 16;
  leafValue |= static_cast<uint64_t>(deltaNumber);

  return leafValue;
}

HARTDictionary::PageType::iterator HARTDictionary::decodeLeafValue(uint64_t leafValue) {
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

  vector<PageType*> pages;
  PageType* page = nullptr;
  const char* endOfPage = nullptr;
  char* dataPtr = nullptr;
  uint16_t deltaNumber = 0;
  const uint64_t pageHeaderSize = sizeof(uint8_t) + 2*sizeof(uint64_t);
  const uint64_t deltaHeaderSize = sizeof(uint8_t) + 3*sizeof(uint64_t);

  const string* deltaRef = nullptr;
  for (const auto& pair : insertValues) {
    if (deltaRef != nullptr) {
      // Will insert delta
      uint64_t prefixSize;
      string deltaValue = delta(*deltaRef, pair.second, prefixSize);
      if (dataPtr != nullptr && dataPtr + deltaHeaderSize + deltaValue.size() > endOfPage) {
        // "Finish" page
        page->endPage(dataPtr);
        pages.push_back(page);
        page = nullptr;
      }
    }

    if (page == nullptr) {
      // Create new page
      cout << "Create page" << endl;
      page = new PageType(pageId++);
      if (pages.size() > 0) {
        pages.back()->nextPage = page;
      }
      dataPtr = page->data;
      endOfPage = page->data + page->size - sizeof(uint8_t);
      deltaNumber = 0;

      deltaRef = &pair.second;

      if (dataPtr + pageHeaderSize + pair.second.size() > endOfPage) {
        cout << "Cannot write one uncompressed string to the page" << endl;
        assert(false);
      }

      page->beginPage(dataPtr);

      // Write uncompressed value
      page->writeId(dataPtr, pair.first);
      page->writeValue(dataPtr, pair.second);

      uint64_t leafValue = encodeLeafValue(page, deltaNumber++);

      bool inserted = false;
      uint64_t* valuePtr = hattrie_get(reverseIndex, pair.second.c_str(), pair.second.size()+1, &inserted);
      *valuePtr = leafValue;
      assert(inserted);

      index.insert(pair.first, leafValue);

      continue;
    }

    // Write delta
    uint64_t prefixSize;
    string deltaValue = delta(*deltaRef, pair.second, prefixSize);
    page->beginDelta(dataPtr);
    page->writeId(dataPtr, pair.first);
    page->writeDelta(dataPtr, *deltaRef, deltaValue, prefixSize);

    uint64_t leafValue = encodeLeafValue(page, deltaNumber++);

    bool inserted = false;
    uint64_t* valuePtr = hattrie_get(reverseIndex, pair.second.c_str(), pair.second.size()+1, &inserted);
    *valuePtr = leafValue;
    assert(inserted);

    index.insert(pair.first, leafValue);
  }

  page->endPage(dataPtr);
  pages.push_back(page);

  return true;
}

bool HARTDictionary::update(uint64_t& id, std::string value) {
  uint64_t leafPtr;
  if (!index.lookup(id, leafPtr)) {
    return false;
  }

  PageType* page = *reinterpret_cast<PageType**>(leafPtr);
  auto iterator = page->getString(id);
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

  auto iterator = decodeLeafValue(*leafPtr);
  assert(iterator);

  id = (*iterator).id;
  return true;
}

bool HARTDictionary::lookup(uint64_t id, std::string& value) {
  uint64_t leafValue;
  if (index.lookup(id, leafValue)) {
    auto iterator = decodeLeafValue(leafValue);
    assert(iterator);

    value = (*iterator).value;
    return true;
  }
  return false;
}
