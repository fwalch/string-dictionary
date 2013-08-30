#include "gtest/gtest.h"
#include "StringDictionary.hpp"
#include "ConstructionStrategies.hpp"
#include "Indexes.hpp"
#include "Pages.hpp"
#include <string>
#include <vector>
#include <iostream>

TEST(Integration, DynamicPage) {
  std::vector<std::string> values {
    "aabc",
    "aabd",
    "baa",
    "bba",
    "ccc",
    "d",
    "db",
  };
  std::vector<std::pair<uint64_t, std::string>> lookupValues;

  StringDictionary<ART, HAT, DynamicPage<1>, OffsetStrategy> dict;
  dict.bulkInsert(values.size(), &values[0]);

  auto callback = [&](uint64_t id, std::string value) {
    lookupValues.push_back(make_pair(id, value));
  };

  for (uint64_t i = 0; i < values.size(); i++) {
    std::string value;
    ASSERT_TRUE(dict.lookup(i+1, value));
    ASSERT_EQ(values[i], value);
  }

  for (uint64_t i = 0; i < values.size(); i++) {
    uint64_t id;
    ASSERT_TRUE(dict.lookup(values[i], id));
    ASSERT_EQ(i+1, id);
  }

  lookupValues.clear();
  dict.rangeLookup("a", callback);
  ASSERT_EQ(2, lookupValues.size());
  ASSERT_EQ("aabc", lookupValues.front().second);
  ASSERT_EQ("aabd", lookupValues.back().second);

  lookupValues.clear();
  dict.rangeLookup("b", callback);
  ASSERT_EQ(2, lookupValues.size());
  ASSERT_EQ("baa", lookupValues.front().second);
  ASSERT_EQ("bba", lookupValues.back().second);

  lookupValues.clear();
  dict.rangeLookup("bb", callback);
  ASSERT_EQ(1, lookupValues.size());
  ASSERT_EQ("bba", lookupValues.front().second);

  lookupValues.clear();
  dict.rangeLookup("cxx", callback);
  ASSERT_EQ(0, lookupValues.size());

  lookupValues.clear();
  dict.rangeLookup("c", callback);
  ASSERT_EQ(1, lookupValues.size());
  ASSERT_EQ("ccc", lookupValues.front().second);

  lookupValues.clear();
  dict.rangeLookup("d", callback);
  ASSERT_EQ(2, lookupValues.size());
  ASSERT_EQ("d", lookupValues.front().second);
  ASSERT_EQ("db", lookupValues.back().second);
}

TEST(Integration, DynamicSlottedPage) {
  std::vector<std::string> values {
    "aabc",
    "aabd",
    "baa",
    "bba",
    "ccc",
    "d",
    "db",
  };
  std::vector<std::pair<uint64_t, std::string>> lookupValues;

  StringDictionary<ART, HAT, DynamicSlottedPage<1>, IndirectStrategy> dict;
  dict.bulkInsert(values.size(), &values[0]);

  auto callback = [&](uint64_t id, std::string value) {
    lookupValues.push_back(make_pair(id, value));
  };

  for (uint64_t i = 0; i < values.size(); i++) {
    std::string value;
    ASSERT_TRUE(dict.lookup(i+1, value));
    ASSERT_EQ(values[i], value);
  }

  for (uint64_t i = 0; i < values.size(); i++) {
    uint64_t id;
    ASSERT_TRUE(dict.lookup(values[i], id));
    ASSERT_EQ(i+1, id);
  }

  lookupValues.clear();
  dict.rangeLookup("a", callback);
  ASSERT_EQ(2, lookupValues.size());
  ASSERT_EQ("aabc", lookupValues.front().second);
  ASSERT_EQ("aabd", lookupValues.back().second);

  lookupValues.clear();
  dict.rangeLookup("b", callback);
  ASSERT_EQ(2, lookupValues.size());
  ASSERT_EQ("baa", lookupValues.front().second);
  ASSERT_EQ("bba", lookupValues.back().second);

  lookupValues.clear();
  dict.rangeLookup("bb", callback);
  ASSERT_EQ(1, lookupValues.size());
  ASSERT_EQ("bba", lookupValues.front().second);

  lookupValues.clear();
  dict.rangeLookup("cxx", callback);
  ASSERT_EQ(0, lookupValues.size());

  lookupValues.clear();
  dict.rangeLookup("c", callback);
  ASSERT_EQ(1, lookupValues.size());
  ASSERT_EQ("ccc", lookupValues.front().second);

  lookupValues.clear();
  dict.rangeLookup("d", callback);
  ASSERT_EQ(2, lookupValues.size());
  ASSERT_EQ("d", lookupValues.front().second);
  ASSERT_EQ("db", lookupValues.back().second);
}

TEST(Integration, SlottedPage) {
  std::vector<std::string> values {
    "aabc",
    "aabd",
    "baa",
    "bba",
    "ccc",
    "d",
    "db",
  };
  std::vector<std::pair<uint64_t, std::string>> lookupValues;

  StringDictionary<ART, HAT, SlottedPage<48>, IndirectStrategy> dict;
  dict.bulkInsert(values.size(), &values[0]);

  auto callback = [&](uint64_t id, std::string value) {
    lookupValues.push_back(make_pair(id, value));
  };

  for (uint64_t i = 0; i < values.size(); i++) {
    std::string value;
    ASSERT_TRUE(dict.lookup(i+1, value));
    ASSERT_EQ(values[i], value);
  }

  for (uint64_t i = 0; i < values.size(); i++) {
    uint64_t id;
    ASSERT_TRUE(dict.lookup(values[i], id));
    ASSERT_EQ(i+1, id);
  }

  lookupValues.clear();
  dict.rangeLookup("a", callback);
  ASSERT_EQ(2, lookupValues.size());
  ASSERT_EQ("aabc", lookupValues.front().second);
  ASSERT_EQ("aabd", lookupValues.back().second);

  lookupValues.clear();
  dict.rangeLookup("b", callback);
  ASSERT_EQ(2, lookupValues.size());
  ASSERT_EQ("baa", lookupValues.front().second);
  ASSERT_EQ("bba", lookupValues.back().second);

  lookupValues.clear();
  dict.rangeLookup("bb", callback);
  ASSERT_EQ(1, lookupValues.size());
  ASSERT_EQ("bba", lookupValues.front().second);

  lookupValues.clear();
  dict.rangeLookup("cxx", callback);
  ASSERT_EQ(0, lookupValues.size());

  lookupValues.clear();
  dict.rangeLookup("c", callback);
  ASSERT_EQ(1, lookupValues.size());
  ASSERT_EQ("ccc", lookupValues.front().second);

  lookupValues.clear();
  dict.rangeLookup("d", callback);
  ASSERT_EQ(2, lookupValues.size());
  ASSERT_EQ("d", lookupValues.front().second);
  ASSERT_EQ("db", lookupValues.back().second);
}

TEST(Integration, BottomUpStrategy) {
  std::vector<std::string> values {
    "aabc",
    "aabd",
    "baa",
    "bba",
    "ccc",
    "d",
    "db",
  };
  std::vector<std::pair<uint64_t, std::string>> lookupValues;

  StringDictionary<ART, SART, BottomUpPage<48>, BottomUpStrategy> dict;
  dict.bulkInsert(values.size(), &values[0]);

  for (uint64_t i = 0; i < values.size(); i++) {
    std::string value;
    ASSERT_TRUE(dict.lookup(i+1, value));
    ASSERT_EQ(values[i], value);
  }

  for (uint64_t i = 0; i < values.size(); i++) {
    uint64_t id;
    ASSERT_TRUE(dict.lookup(values[i], id));
    ASSERT_EQ(i+1, id);
  }
}
