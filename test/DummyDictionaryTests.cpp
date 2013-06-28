#include "gtest/gtest.h"
#include "DummyDictionary.hpp"

// Test only that no errors occur

TEST(DummyDictionary, Insert) {
  DummyDictionary dict;

  dict.insert("value");
}

TEST(DummyDictionary, BulkInsert) {
  DummyDictionary dict;
  std::string values[] = {
    "a",
    "b"
  };

  dict.bulkInsert(2, values);
}

TEST(DummyDictionary, LookupByID) {
  DummyDictionary dict;

  std::string value;
  ASSERT_TRUE(dict.lookup(0, value));
}

TEST(DummyDictionary, LookupByValue) {
  DummyDictionary dict;

  uint64_t id;
  ASSERT_TRUE(dict.lookup("value", id));
}

TEST(DummyDictionary, Update) {
  DummyDictionary dict;

  uint64_t id = 0;
  ASSERT_TRUE(dict.update(id, "newValue"));
}
