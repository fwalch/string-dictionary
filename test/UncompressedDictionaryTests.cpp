#include "gtest/gtest.h"
#include "UncompressedDictionary.hpp"

TEST(UncompressedDictionary, Integrity) {
  UncompressedDictionary dict;

  ASSERT_EQ(0, dict.size());

  dict.insert("value");
  ASSERT_EQ(1, dict.size());
}
