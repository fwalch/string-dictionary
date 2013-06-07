#include <unordered_map>
#include <string>
#include "gtest/gtest.h"
#include "IndexART.hpp"
#include "ReverseIndexART.hpp"

#define VALUES 100

using namespace std;

TEST(IndexART, Integrity) {
  IndexART art;

  for (uint64_t i = 0; i < VALUES; i++) {
    art.insert(i, "Value " + to_string(i));
  }

  for (uint64_t i = 0; i < VALUES; i++) {
    string value;
    ASSERT_TRUE(art.lookup(i, value));
    ASSERT_EQ("Value " + to_string(i), value);
  }
}

TEST(ReverseIndexARTWithIndexART, Integrity) {
  IndexART cArt;
  ReverseIndexART<IndexART> art(cArt);

  for (uint64_t i = 0; i < VALUES; i++) {
    cArt.insert(i, "Key " + to_string(i));
    art.insert("Key " + to_string(i), i);
  }

  for (uint64_t i = 0; i < VALUES; i++) {
    uint64_t value;
    ASSERT_TRUE(art.lookup("Key " + to_string(i), value));
    ASSERT_EQ(i, value);
  }
}

TEST(ReverseIndexARTWithMap, Integrity) {
  typedef unordered_map<uint64_t, string> mapType;
  mapType map;
  ReverseIndexART<mapType> art(map);

  for (uint64_t i = 0; i < VALUES; i++) {
    map.insert(make_pair(i, "Key " + to_string(i)));
    art.insert("Key " + to_string(i), i);
  }

  for (uint64_t i = 0; i < VALUES; i++) {
    uint64_t value;
    ASSERT_TRUE(art.lookup("Key " + to_string(i), value));
    ASSERT_EQ(i, value);
  }
}
