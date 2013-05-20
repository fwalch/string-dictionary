#include "HashARTDictionary.hpp"

using namespace std;

void HashARTDictionary::insert(string value) {
  unsigned long sid;
  if (!reverseIndex.lookup(value, sid)) {
    // String not in dictionary
    reverseIndex.insert(value, nextId);
    index.insert(make_pair(nextId, value));
    nextId++;
  }
}
