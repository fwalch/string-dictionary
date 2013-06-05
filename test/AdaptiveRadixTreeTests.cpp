#include "gtest/gtest.h"
#include "IndexART.hpp"
#include "ReverseIndexART.hpp"

#define VALUES 100

using namespace std;

TEST(IndexART, Integrity) {
  IndexART art;

  for (unsigned long i = 0; i < VALUES; i++) {
    art.insert(i, "Value " + to_string(i));
  }

  for (unsigned long i = 0; i < VALUES; i++) {
    string value;
    ASSERT_TRUE(art.lookup(i, value));
    ASSERT_EQ("Value " + to_string(i), value);
  }
}

TEST(ReverseIndexART, Integrity) {
  IndexART cArt;
  IAReverseIndexART art(cArt);

  for (unsigned long i = 0; i < VALUES; i++) {
    cArt.insert(i, "Key " + to_string(i));
    art.insert("Key " + to_string(i), i);
  }

  for (unsigned long i = 0; i < VALUES; i++) {
    unsigned long value;
    ASSERT_TRUE(art.lookup("Key " + to_string(i), value));
    ASSERT_EQ(i, value);
  }
}
