#include "gtest/gtest.h"
#include "ART.hpp"
#include <string>
#include <vector>

class NoLeafStore : public LeafStore {
  public:
  std::vector<std::string> values;
 
  NoLeafStore() {
    values = {
      "aabc",
      "abbd",
      "baa",
      "bab"
    };
  }
  std::pair<uint64_t, std::string> getLeaf(uint64_t leafValue) const {
    return std::make_pair(leafValue, values[leafValue-1]);
  }
};

TEST(ART, RangeLookup) {
  NoLeafStore store;
  ART<std::string> art(&store);

  for (size_t i = 0; i < store.values.size(); i++) {
    art.insert(store.values[i], i+1);
  }

  auto range = art.rangeLookup("a");
  ASSERT_EQ(range.first, 1);
  ASSERT_EQ(range.second, 2);

  range = art.rangeLookup("ab");
  ASSERT_EQ(range.first, 2);
  ASSERT_EQ(range.second, 2);

  range = art.rangeLookup("b");
  ASSERT_EQ(range.first, 3);
  ASSERT_EQ(range.second, 4);
}
