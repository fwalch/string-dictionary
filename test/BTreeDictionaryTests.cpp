#include "gtest/gtest.h"
#include "BTreeDictionary.hpp"

TEST(BTreeDictionary, Insert) {
  BTreeDictionary dict;

  ASSERT_EQ(0, dict.size());

  ASSERT_EQ(0, dict.insert("value"));
  ASSERT_EQ(1, dict.size());
}

TEST(BTreeDictionary, BulkInsert) {
  BTreeDictionary dict;
  std::string values[] = {
    "a",
    "b"
  };

  ASSERT_EQ(0, dict.size());
  ASSERT_TRUE(dict.bulkInsert(2, values));
  ASSERT_EQ(2, dict.size());
}

TEST(BTreeDictionary, InsertDuplicate) {
  BTreeDictionary dict;

  uint64_t id;
  id = dict.insert("value");
  ASSERT_EQ(id, dict.insert("value"));

  ASSERT_EQ(1, dict.size());
}

TEST(BTreeDictionary, LookupByValue) {
  BTreeDictionary dict;

  uint64_t id = dict.insert("value");
  uint64_t lookupId;

  ASSERT_TRUE(dict.lookup("value", lookupId));
  ASSERT_EQ(id, lookupId);
}

TEST(BTreeDictionary, LookupByNonexistentValue) {
  BTreeDictionary dict;

  uint64_t lookupId;
  ASSERT_FALSE(dict.lookup("value", lookupId));
}

TEST(BTreeDictionary, LookupByID) {
  BTreeDictionary dict;

  uint64_t id = dict.insert("value");
  std::string lookupValue;

  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("value", lookupValue);
}

TEST(BTreeDictionary, LookupByNonexistentID) {
  BTreeDictionary dict;

  std::string lookupValue;
  ASSERT_FALSE(dict.lookup(0, lookupValue));
}

TEST(BTreeDictionary, Update) {
  BTreeDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.update(id, "newValue"));

  std::string lookupValue;
  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("newValue", lookupValue);
}

TEST(BTreeDictionary, UpdateDuplicate) {
  BTreeDictionary dict;

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

TEST(BTreeDictionary, UpdateNonexistent) {
  BTreeDictionary dict;

  uint64_t id = 0;
  ASSERT_FALSE(dict.update(id, "newValue"));
}

/*
TEST(BTreeDictionary, Remove) {
  BTreeDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(BTreeDictionary, RemoveDuplicate) {
  BTreeDictionary dict;

  uint64_t id = dict.insert("value");
  dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(1, dict.size());

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(BTreeDictionary, RemoveNonexistent) {
  BTreeDictionary dict;

  ASSERT_FALSE(dict.remove(0));
}
*/
