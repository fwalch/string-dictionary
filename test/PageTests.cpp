#include "gtest/gtest.h"
#include "Page.hpp"
#include <vector>

using namespace std;

inline string delta(const string& ref, const string& value, uint64_t& pos) {
  pos = 0;
  while (pos < ref.size() && pos < value.size() && ref[pos] == value[pos]) {
    pos++;
  }

  auto delta = value.substr(pos, value.size()-pos);
  cout << "Delta between " << ref << " & " << value << ":" << delta << endl;
  return delta;
}

TEST(Page, Create) {
  vector<string> values {
    "aaa",
    "aab",
    "aac",
    "baa",
    "bab",
    "bba",
    "cba",
  };
  const uint64_t size = values.size();
  vector<pair<uint64_t, string>> insertValues;
  insertValues.reserve(size);

  uint64_t nextId = 0;
  for (size_t i = 0; i < size; i++) {
    insertValues.push_back(make_pair(nextId++, values[i]));
  }

  const uint16_t frequency = 3;
  typedef Page<1024, frequency> pageType;
  vector<pageType*> pages;
  pageType* page = nullptr;
  const char* endOfPage = nullptr;
  char* dataPtr = nullptr;
  const uint64_t prefixHeaderSize = sizeof(uint8_t) + 2*sizeof(uint64_t);
  const uint64_t deltaHeaderSize = sizeof(uint8_t) + 3*sizeof(uint64_t);

  uint64_t i = 0;
  const string* deltaRef = nullptr;
  for (const auto& pair : insertValues) {
    if (i%frequency == 0) {
      // Will insert full string
      if (dataPtr != nullptr && dataPtr + prefixHeaderSize + pair.second.size() > endOfPage) {
        // "Finish" page
        page->endPage(dataPtr);
        pages.push_back(page);
        page = nullptr;
      }
    }
    else {
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
      page = new pageType();
      if (pages.size() > 0) {
        pages.back()->nextPage = page;
      }
      dataPtr = page->data;
      endOfPage = page->data + page->size - sizeof(uint8_t);
      i = 0;
    }

    if (i%frequency==0) {
      page->startPrefix(dataPtr);
      page->writeId(dataPtr, pair.first);
      page->writeValue(dataPtr, pair.second);
      deltaRef = &pair.second;
    }
    else {
      assert(deltaRef != nullptr);
      uint64_t prefixSize;
      string deltaValue = delta(*deltaRef, pair.second, prefixSize);

      page->startDelta(dataPtr);
      page->writeId(dataPtr, pair.first);
      page->writeDelta(dataPtr, *deltaRef, deltaValue, prefixSize);
    }

    i++;
  }
  page->endPage(dataPtr);
  pages.push_back(page);

  page = pages.front();

  for (auto iterator = page->getString(0, 0); iterator; ++iterator) {
    auto leaf = *iterator;
    cout << leaf.id << " " << leaf.value << endl;
  }
}
