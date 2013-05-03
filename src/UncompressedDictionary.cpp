#include "UncompressedDictionary.hpp"

using namespace std;

void UncompressedDictionary::insert(string value) {
  index.insert(make_pair(++nextId, value));
  reverseIndex.insert(make_pair(value, nextId));
}
