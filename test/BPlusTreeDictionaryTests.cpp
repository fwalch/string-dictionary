#include "gtest/gtest.h"
#include "BPlusTreeDictionary.hpp"

TEST(BPlusTreeDictionary, Insert) {
  BPlusTreeDictionary dict;

  ASSERT_EQ(0, dict.size());

  ASSERT_EQ(0, dict.insert("value"));
  ASSERT_EQ(1, dict.size());
}

TEST(BPlusTreeDictionary, BulkInsert) {
  BPlusTreeDictionary dict;
  std::string values[] = {
    "a",
    "b"
  };

  ASSERT_EQ(0, dict.size());
  ASSERT_TRUE(dict.bulkInsert(2, values));
  ASSERT_EQ(2, dict.size());
}

TEST(BPlusTreeDictionary, InsertDuplicate) {
  BPlusTreeDictionary dict;

  uint64_t id;
  id = dict.insert("value");
  ASSERT_EQ(id, dict.insert("value"));

  ASSERT_EQ(1, dict.size());
}

TEST(BPlusTreeDictionary, LookupByValue) {
  BPlusTreeDictionary dict;

  uint64_t id = dict.insert("value");
  uint64_t lookupId;

  ASSERT_TRUE(dict.lookup("value", lookupId));
  ASSERT_EQ(id, lookupId);
}

TEST(BPlusTreeDictionary, LookupByNonexistentValue) {
  BPlusTreeDictionary dict;

  uint64_t lookupId;
  ASSERT_FALSE(dict.lookup("value", lookupId));
}

TEST(BPlusTreeDictionary, LookupByID) {
  BPlusTreeDictionary dict;

  uint64_t id = dict.insert("value");
  std::string lookupValue;

  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("value", lookupValue);
}

TEST(BPlusTreeDictionary, LookupByNonexistentID) {
  BPlusTreeDictionary dict;

  std::string lookupValue;
  ASSERT_FALSE(dict.lookup(0, lookupValue));
}

TEST(BPlusTreeDictionary, Update) {
  BPlusTreeDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.update(id, "newValue"));

  std::string lookupValue;
  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("newValue", lookupValue);
}

TEST(BPlusTreeDictionary, UpdateDuplicate) {
  BPlusTreeDictionary dict;

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

TEST(BPlusTreeDictionary, UpdateNonexistent) {
  BPlusTreeDictionary dict;

  uint64_t id = 0;
  ASSERT_FALSE(dict.update(id, "newValue"));
}

/*
TEST(BPlusTreeDictionary, Remove) {
  BPlusTreeDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(BPlusTreeDictionary, RemoveDuplicate) {
  BPlusTreeDictionary dict;

  uint64_t id = dict.insert("value");
  dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(1, dict.size());

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(BPlusTreeDictionary, RemoveNonexistent) {
  BPlusTreeDictionary dict;

  ASSERT_FALSE(dict.remove(0));
}
*/
