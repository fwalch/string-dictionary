#include "gtest/gtest.h"
#include "ARTDictionary.hpp"

TEST(ARTDictionary, Integrity) {
  ARTDictionary dict;

  ASSERT_EQ(0, dict.size());

  dict.insert("value");
  ASSERT_EQ(1, dict.size());
}
