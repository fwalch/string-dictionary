#include "BottomUpStrategy.hpp"

template<class TIdIndex, class TStringIndex, class TLeaf>
PageIterator<TLeaf> BottomUpStrategy<TIdIndex, TStringIndex, TLeaf>::decodeLeaf(uint64_t leafValue) const {
  TLeaf* leaf = reinterpret_cast<TLeaf*>(leafValue >> 16);
  uint16_t offset = leafValue & 0xFFFF;

  if (offset == 1) {
    // Special value; get last value in leaf
    return leaf->last();
  }

  return leaf->getByOffset(offset);
}

template<class TIdIndex, class TStringIndex, class TLeaf>
PageIterator<TLeaf> BottomUpStrategy<TIdIndex, TStringIndex, TLeaf>::decodeLeaf(uint64_t leafValue, std::string lookupValue) const {
  return reinterpret_cast<TLeaf*>(leafValue >> 16)->find(lookupValue);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
template<class TIdIndex, class TStringIndex, class TLeaf>
void BottomUpStrategy<TIdIndex, TStringIndex, TLeaf>::leafCallback(TLeaf* leaf, uint16_t deltaNumber, uint16_t offset, uint64_t id, std::string value) {
  if (deltaNumber == 0) {
    uint64_t leafValue = this->encodeLeaf(leaf, 1);
    this->reverseIndex.insert(value, leafValue);
  }
  else if (deltaNumber == 1) {
    uint64_t leafValue = this->encodeLeaf(leaf, 0);
    this->reverseIndex.insert(value, leafValue);

    // Use the offset for the ID-to-string index
    leafValue = this->encodeLeaf(leaf, offset);
    this->index.insert(id, leafValue);
  }
  else if (deltaNumber == 2) {
    // Use the offset for the ID-to-string index
    uint64_t leafValue = this->encodeLeaf(leaf, offset);
    this->index.insert(id, leafValue);
  }
}
#pragma GCC diagnostic pop

template<class TIdIndex, class TStringIndex, class TLeaf>
bool BottomUpStrategy<TIdIndex, TStringIndex, TLeaf>::rangeLookup(std::string prefix, PageIterator<TLeaf>& start, PageIterator<TLeaf>& end) const {
  std::pair<uint64_t, uint64_t> range = this->reverseIndex.rangeLookup(prefix);

  if (range.first == 0) {
    // Prefix not found
    return false;
  }

  start = reinterpret_cast<TLeaf*>(range.first >> 16)->firstPrefix(prefix);

  if (!start) {
    // Prefix not found
    return false;
  }

  end = reinterpret_cast<TLeaf*>(range.second >> 16)->lastPrefix(prefix);

#ifdef DEBUG
  assert(range.second != 0);
  assert(end);
#endif

  return true;
  throw;
}
