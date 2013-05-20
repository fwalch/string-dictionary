#include "ARTDictionary.hpp"

using namespace std;

void ARTDictionary::insert(string value) {
  unsigned long sid;
  if (!reverseIndex.lookup(value, sid)) {
    // String not in dictionary
    reverseIndex.insert(value, nextId);
    index.insert(nextId, value);
    nextId++;
  }
}
