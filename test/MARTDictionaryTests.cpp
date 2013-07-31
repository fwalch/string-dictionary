#include "gtest/gtest.h"
#include "MARTDictionary.hpp"

TEST(MARTDictionary, Insert) {
  MARTDictionary dict;

  ASSERT_EQ(0, dict.size());

  ASSERT_EQ(0, dict.insert("value"));
  ASSERT_EQ(1, dict.size());
}

TEST(MARTDictionary, BulkInsert) {
  MARTDictionary dict;
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
  }
}

TEST(MARTDictionary, InsertDuplicate) {
  MARTDictionary dict;

  uint64_t id;
  id = dict.insert("value");
  ASSERT_EQ(id, dict.insert("value"));

  ASSERT_EQ(1, dict.size());
}

TEST(MARTDictionary, LookupByValue) {
  MARTDictionary dict;

  uint64_t id = dict.insert("value");
  uint64_t lookupId;

  ASSERT_TRUE(dict.lookup("value", lookupId));
  ASSERT_EQ(id, lookupId);
}

TEST(MARTDictionary, LookupByNonexistentValue) {
  MARTDictionary dict;

  uint64_t lookupId;
  ASSERT_FALSE(dict.lookup("value", lookupId));
}

TEST(MARTDictionary, LookupByID) {
  MARTDictionary dict;

  uint64_t id = dict.insert("value");
  std::string lookupValue;

  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("value", lookupValue);
}

TEST(MARTDictionary, LookupByNonexistentID) {
  MARTDictionary dict;

  std::string lookupValue;
  ASSERT_FALSE(dict.lookup(0, lookupValue));
}

TEST(MARTDictionary, Update) {
  MARTDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.update(id, "newValue"));

  std::string lookupValue;
  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("newValue", lookupValue);
}

TEST(MARTDictionary, UpdateDuplicate) {
  MARTDictionary dict;

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

TEST(MARTDictionary, UpdateNonexistent) {
  MARTDictionary dict;

  uint64_t id = 0;
  ASSERT_FALSE(dict.update(id, "newValue"));
}

/*
TEST(MARTDictionary, Remove) {
  MARTDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(MARTDictionary, RemoveDuplicate) {
  MARTDictionary dict;

  uint64_t id = dict.insert("value");
  dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(1, dict.size());

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(MARTDictionary, RemoveNonexistent) {
  MARTDictionary dict;

  ASSERT_FALSE(dict.remove(0));
}
*/
