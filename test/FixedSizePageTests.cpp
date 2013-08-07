#include "gtest/gtest.h"
#include "FixedSizePage.hpp"
#include <vector>

using namespace std;

inline string delta(const string& ref, const string& value) {
  uint64_t pos = 0;
  while (pos < ref.size() && pos < value.size() && ref[pos] == value[pos]) {
    pos++;
  }

  auto delta = value.substr(pos, value.size()-pos);
  cout << "Delta between " << ref << " & " << value << ":" << delta << endl;
  return delta;
}

TEST(FixedSizePage, Create) {
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

  typedef FixedSizePage<64> pageType;
  vector<pageType*> pages;
  pageType* page = nullptr;
  const char* endOfPage = nullptr;
  char* dataPtr = nullptr;
  const uint64_t pageHeaderSize = sizeof(uint8_t) + 2*sizeof(uint64_t);
  const uint64_t deltaHeaderSize = sizeof(uint8_t) + 3*sizeof(uint64_t);

  const string* deltaRef = nullptr;
  for (const auto& pair : insertValues) {
    if (deltaRef != nullptr) {
      // Will insert delta
      string deltaValue = delta(*deltaRef, pair.second);
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

      deltaRef = &pair.second;

      if (dataPtr + pageHeaderSize + pair.second.size() > endOfPage) {
        cout << "Cannot write one uncompressed string to the page" << endl;
        assert(false);
      }

      page->beginPage(dataPtr);

      // Write uncompressed value
      page->writeId(dataPtr, pair.first);
      page->writeValue(dataPtr, pair.second);

      continue;
    }

    // Write delta
    string deltaValue = delta(*deltaRef, pair.second);
    page->beginDelta(dataPtr);
    page->writeId(dataPtr, pair.first);
    page->writeDelta(dataPtr, *deltaRef, deltaValue);
  }
  page->endPage(dataPtr);
  pages.push_back(page);

  page = pages.front();

  for (auto iterator = page->getString(0, 0); iterator; ++iterator) {
    auto leaf = *iterator;
    cout << leaf.id << " " << leaf.value << endl;
  }
}
