#include "UncompressedDictionary.hpp"

using namespace std;

void UncompressedDictionary::insert(string value) {
  auto reverseIt = reverseIndex.find(value);
  if (reverseIt == reverseIndex.end()) {
    // String not in dictionary
    index.insert(make_pair(nextId, value));
    reverseIndex.insert(make_pair(value, nextId));
    nextId++;
  }
}
