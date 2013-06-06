#include "gtest/gtest.h"
#include "ARTDictionary.hpp"

TEST(ARTDictionary, Insert) {
  ARTDictionary dict;

  ASSERT_EQ(0, dict.size());

  ASSERT_EQ(0, dict.insert("value"));
  ASSERT_EQ(1, dict.size());
}

TEST(ARTDictionary, InsertDuplicate) {
  ARTDictionary dict;

  uint64_t id;
  id = dict.insert("value");
  ASSERT_EQ(id, dict.insert("value"));

  ASSERT_EQ(1, dict.size());
}

TEST(ARTDictionary, LookupByValue) {
  ARTDictionary dict;

  uint64_t id = dict.insert("value");
  uint64_t lookupId;

  ASSERT_TRUE(dict.lookup("value", lookupId));
  ASSERT_EQ(id, lookupId);
}

TEST(ARTDictionary, LookupByNonexistentValue) {
  ARTDictionary dict;

  uint64_t lookupId;
  ASSERT_FALSE(dict.lookup("value", lookupId));
}

TEST(ARTDictionary, LookupByID) {
  ARTDictionary dict;

  uint64_t id = dict.insert("value");
  std::string lookupValue;

  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("value", lookupValue);
}

TEST(ARTDictionary, LookupByNonexistentID) {
  ARTDictionary dict;

  std::string lookupValue;
  ASSERT_FALSE(dict.lookup(0, lookupValue));
}

TEST(ARTDictionary, Update) {
  ARTDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.update(id, "newValue"));

  std::string lookupValue;
  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("newValue", lookupValue);
}

TEST(ARTDictionary, UpdateDuplicate) {
  ARTDictionary dict;

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

TEST(ARTDictionary, UpdateNonexistent) {
  ARTDictionary dict;

  uint64_t id = 0;
  ASSERT_FALSE(dict.update(id, "newValue"));
}

/*
TEST(ARTDictionary, Remove) {
  ARTDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(ARTDictionary, RemoveDuplicate) {
  ARTDictionary dict;

  uint64_t id = dict.insert("value");
  dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(1, dict.size());

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(ARTDictionary, RemoveNonexistent) {
  ARTDictionary dict;

  ASSERT_FALSE(dict.remove(0));
}
*/
