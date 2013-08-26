#include "gtest/gtest.h"
#include "MultiUncompressedPage.hpp"
#include "SingleUncompressedPage.hpp"
#include "DynamicPage.hpp"
#include <vector>

using namespace std;

TEST(MultipleUncompressedStringsPerPage, Create) {
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

  typedef MultiUncompressedPage<1024, 3> pageType;

  std::vector<pageType*> pages;
  pageType::Loader loader;
  loader.load(insertValues, [&pages](pageType* page, uint16_t deltaValue, uint64_t id, std::string value) {
      pages.push_back(page);
  });

  uint64_t i = 0;
  for (auto iterator = pages.front()->getId(0); iterator; ++iterator) {
    auto leaf = *iterator;
    cout << leaf.id << " " << leaf.value << endl;
    ASSERT_EQ(insertValues[i].first, leaf.id);
    ASSERT_EQ(insertValues[i].second, leaf.value);
    i++;
  }
  ASSERT_EQ(i, values.size());
}

TEST(SingleUncompressedStringPerPage, Create) {
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

  typedef SingleUncompressedPage<1024> pageType;

  std::vector<pageType*> pages;
  pageType::Loader loader;
  loader.load(insertValues, [&pages](pageType* page, uint16_t delta, uint64_t id, std::string value) {
      pages.push_back(page);
  });

  uint64_t i = 0;
  for (auto iterator = pages.front()->getId(0); iterator; ++iterator) {
    auto leaf = *iterator;
    cout << leaf.id << " " << leaf.value << endl;
    ASSERT_EQ(insertValues[i].first, leaf.id);
    ASSERT_EQ(insertValues[i].second, leaf.value);
    i++;
  }
  ASSERT_EQ(i, values.size());
}

TEST(DynamicPage, Create) {
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

  typedef DynamicPage<> pageType;
  pageType* firstPage = nullptr;

  pageType::Loader loader;
  loader.load(insertValues, [&firstPage](pageType* page, uint16_t deltaValue, uint64_t id, std::string value) {
    if (firstPage == nullptr) {
      firstPage = page;
    }
  });

  uint64_t i = 0;
  for (auto iterator = firstPage->getId(0); iterator; ++iterator) {
    auto leaf = *iterator;
    cout << leaf.id << " " << leaf.value << endl;
    ASSERT_EQ(insertValues[i].first, leaf.id);
    ASSERT_EQ(insertValues[i].second, leaf.value);
    i++;
  }
  ASSERT_EQ(i, values.size());
}
