#include "gtest/gtest.h"
#include "SimpleDictionary.hpp"

TEST(SimpleDictionary, Integrity) {
  SimpleDictionary dict;

  ASSERT_EQ(0, dict.size());

  dict.insert("value");
  ASSERT_EQ(1, dict.size());
}
