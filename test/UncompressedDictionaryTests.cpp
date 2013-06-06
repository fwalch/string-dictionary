#include "gtest/gtest.h"
#include "UncompressedDictionary.hpp"

TEST(UncompressedDictionary, Insert) {
  UncompressedDictionary dict;

  ASSERT_EQ(0, dict.size());

  ASSERT_EQ(0, dict.insert("value"));
  ASSERT_EQ(1, dict.size());
}

TEST(UncompressedDictionary, InsertDuplicate) {
  UncompressedDictionary dict;

  uint64_t id;
  id = dict.insert("value");
  ASSERT_EQ(id, dict.insert("value"));

  ASSERT_EQ(1, dict.size());
}

TEST(UncompressedDictionary, LookupByValue) {
  UncompressedDictionary dict;

  uint64_t id = dict.insert("value");
  uint64_t lookupId;

  ASSERT_TRUE(dict.lookup("value", lookupId));
  ASSERT_EQ(id, lookupId);
}

TEST(UncompressedDictionary, LookupByNonexistentValue) {
  UncompressedDictionary dict;

  uint64_t lookupId;
  ASSERT_FALSE(dict.lookup("value", lookupId));
}

TEST(UncompressedDictionary, LookupByID) {
  UncompressedDictionary dict;

  uint64_t id = dict.insert("value");
  std::string lookupValue;

  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("value", lookupValue);
}

TEST(UncompressedDictionary, LookupByNonexistentID) {
  UncompressedDictionary dict;

  std::string lookupValue;
  ASSERT_FALSE(dict.lookup(0, lookupValue));
}

TEST(UncompressedDictionary, Update) {
  UncompressedDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.update(id, "newValue"));

  std::string lookupValue;
  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("newValue", lookupValue);
}

TEST(UncompressedDictionary, UpdateDuplicate) {
  UncompressedDictionary dict;

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

TEST(UncompressedDictionary, UpdateNonexistent) {
  UncompressedDictionary dict;

  uint64_t id = 0;
  ASSERT_FALSE(dict.update(id, "newValue"));
}

/*
TEST(UncompressedDictionary, Remove) {
  UncompressedDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(UncompressedDictionary, RemoveDuplicate) {
  UncompressedDictionary dict;

  uint64_t id = dict.insert("value");
  dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(1, dict.size());

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(UncompressedDictionary, RemoveNonexistent) {
  UncompressedDictionary dict;

  ASSERT_FALSE(dict.remove(0));
}
*/
