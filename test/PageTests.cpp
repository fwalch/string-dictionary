#include "gtest/gtest.h"
#include "MultiUncompressedPage.hpp"
#include "SingleUncompressedPage.hpp"
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
  loader.load(insertValues, [&pages](pageType* page, std::string value, uint64_t id) {
      pages.push_back(page);
  });

  for (auto iterator = pages.front()->getId(0); iterator; ++iterator) {
    auto leaf = *iterator;
    cout << leaf.id << " " << leaf.value << endl;
  }
  EXPECT_TRUE(false);
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
  loader.load(insertValues, [&pages](pageType* page, uint16_t delta, std::string value, uint64_t id) {
      pages.push_back(page);
  });

  for (auto iterator = pages.front()->getId(0); iterator; ++iterator) {
    auto leaf = *iterator;
    cout << leaf.id << " " << leaf.value << endl;
  }
  EXPECT_TRUE(false);
}
