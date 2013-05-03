#include "gtest/gtest.h"
#include "DummyDictionary.hpp"

TEST(DummyDictionary, Integrity) {
  DummyDictionary dict;

  ASSERT_EQ(0, dict.size());

  // Should not store any values
  dict.insert("value");
  ASSERT_EQ(0, dict.size());
}
