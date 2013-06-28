#include "gtest/gtest.h"
#include "SimpleDictionary.hpp"

TEST(SimpleDictionary, Insert) {
  SimpleDictionary dict;

  ASSERT_EQ(0, dict.size());

  ASSERT_EQ(0, dict.insert("value"));
  ASSERT_EQ(1, dict.size());
}

TEST(SimpleDictionary, BulkInsert) {
  SimpleDictionary dict;
  std::string values[] = {
    "a",
    "b"
  };

  ASSERT_EQ(0, dict.size());
  ASSERT_TRUE(dict.bulkInsert(2, values));
  ASSERT_EQ(2, dict.size());
}

TEST(SimpleDictionary, InsertDuplicate) {
  SimpleDictionary dict;

  uint64_t id;
  id = dict.insert("value");
  ASSERT_EQ(id, dict.insert("value"));

  ASSERT_EQ(1, dict.size());
}

TEST(SimpleDictionary, LookupByValue) {
  SimpleDictionary dict;

  uint64_t id = dict.insert("value");
  uint64_t lookupId;

  ASSERT_TRUE(dict.lookup("value", lookupId));
  ASSERT_EQ(id, lookupId);
}

TEST(SimpleDictionary, LookupByNonexistentValue) {
  SimpleDictionary dict;

  uint64_t lookupId;
  ASSERT_FALSE(dict.lookup("value", lookupId));
}

TEST(SimpleDictionary, LookupByID) {
  SimpleDictionary dict;

  uint64_t id = dict.insert("value");
  std::string lookupValue;

  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("value", lookupValue);
}

TEST(SimpleDictionary, LookupByNonexistentID) {
  SimpleDictionary dict;

  std::string lookupValue;
  ASSERT_FALSE(dict.lookup(0, lookupValue));
}

TEST(SimpleDictionary, Update) {
  SimpleDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.update(id, "newValue"));

  std::string lookupValue;
  ASSERT_TRUE(dict.lookup(id, lookupValue));
  ASSERT_EQ("newValue", lookupValue);
}

TEST(SimpleDictionary, UpdateDuplicate) {
  SimpleDictionary dict;

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

TEST(SimpleDictionary, UpdateNonexistent) {
  SimpleDictionary dict;

  uint64_t id = 0;
  ASSERT_FALSE(dict.update(id, "newValue"));
}

/*
TEST(SimpleDictionary, Remove) {
  SimpleDictionary dict;

  uint64_t id = dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(SimpleDictionary, RemoveDuplicate) {
  SimpleDictionary dict;

  uint64_t id = dict.insert("value");
  dict.insert("value");

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(1, dict.size());

  ASSERT_TRUE(dict.remove(id));
  ASSERT_EQ(0, dict.size());
}

TEST(SimpleDictionary, RemoveNonexistent) {
  SimpleDictionary dict;

  ASSERT_FALSE(dict.remove(0));
}
*/
