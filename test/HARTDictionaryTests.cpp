#include "gtest/gtest.h"
#include "HARTDictionary.hpp"
#include <iostream>

using namespace std;

/*TEST(HARTDictionary, Insert) {
  HARTDictionary dict;

  ASSERT_EQ(0, dict.size());

  ASSERT_EQ(0, dict.insert("value"));
  ASSERT_EQ(1, dict.size());
}
*/

TEST(HARTDictionary, BulkInsert) {
  HARTDictionary<> dict;
  std::string values[] = {
    "aa",
    "ab",
    "abc",
    "cd"
  };

  ASSERT_EQ(0, dict.size());
  ASSERT_TRUE(dict.bulkInsert(4, values));
  ASSERT_EQ(4, dict.size());

  for (size_t i = 0; i < 4; i++) {
    uint64_t id;
    ASSERT_TRUE(dict.lookup(values[i], id));
    ASSERT_EQ(i, id);

    string value;
    ASSERT_TRUE(dict.lookup(id, value));
    ASSERT_EQ(values[i], value);
  }
}

/*TEST(HARTDictionary, InsertDuplicate) {
  HARTDictionary dict;

  uint64_t id;
  id = dict.insert("value");
  ASSERT_EQ(id, dict.insert("value"));

  ASSERT_EQ(1, dict.size());
}

TEST(HARTDictionary, LookupByValue) {
  HARTDictionary dict;

  uint64_t id = dict.insert("value");
  uint64_t lookupId;

  ASSERT_TRUE(dict.lookup("value", lookupId));
  ASSERT_EQ(id, lookupId);
}

TEST(HARTDictionary, LookupByNonexistentValue) {
  HARTDictionary dict;

  uint64_t lookupId;
  ASSERT_FALSE(dict.lookup("value", lookupId));
}

TEST(HARTDictionary, LookupByID) {
  HARTDictionary dict;

  uint64_t id = dict.insert("value");
  std::string lookupValue;

  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("value", lookupValue);
}

TEST(HARTDictionary, LookupByNonexistentID) {
  HARTDictionary dict;

  std::string lookupValue;
  ASSERT_FALSE(dict.lookup(0, lookupValue));
}

TEST(HARTDictionary, Update) {
  HARTDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.update(id, "newValue"));

  std::string lookupValue;
  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("newValue", lookupValue);
}

TEST(HARTDictionary, UpdateDuplicate) {
  HARTDictionary dict;

  uint64_t id = dict.insert("value");
  uint64_t id2 = dict.insert("value");

  ASSERT_TRUE(dict.update(id, "newValue"));
  ASSERT_NE(id2, id);
  ASSERT_EQ(2, dict.size());

  std::string lookupValue;
  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("newValue", lookupValue);

  ASSERT_TRUE(dict.lookup(id2, lookupValue));
  ASSERT_EQ("value", lookupValue);
}

TEST(HARTDictionary, UpdateNonexistent) {
  HARTDictionary dict;

  uint64_t id = 0;
  ASSERT_FALSE(dict.update(id, "newValue"));
}*/

/*
TEST(HARTDictionary, Remove) {
  HARTDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(HARTDictionary, RemoveDuplicate) {
  HARTDictionary dict;

  uint64_t id = dict.insert("value");
  dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(1, dict.size());

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(HARTDictionary, RemoveNonexistent) {
  HARTDictionary dict;

  ASSERT_FALSE(dict.remove(0));
}
*/
