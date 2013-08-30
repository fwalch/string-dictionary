#include "gtest/gtest.h"
#include "HAT.hpp"
#include <string>
#include <vector>

TEST(HAT, RangeLookup) {
  std::vector<std::string> values = {
    "aabc",
    "abbd",
    "baa",
    "bab"
  };
  HAT<std::string> hat;

  for (size_t i = 0; i < values.size(); i++) {
    hat.insert(values[i], i+1);
  }

  auto range = hat.rangeLookup("a");
  ASSERT_EQ(range.first, 1);
  ASSERT_EQ(range.second, 2);

  range = hat.rangeLookup("ab");
  ASSERT_EQ(range.first, 2);
  ASSERT_EQ(range.second, 2);

  range = hat.rangeLookup("b");
  ASSERT_EQ(range.first, 3);
  ASSERT_EQ(range.second, 4);
}

