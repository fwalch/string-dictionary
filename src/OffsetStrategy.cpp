#include "OffsetStrategy.hpp"

template<class TIdIndex, class TStringIndex, class TLeaf>
PageIterator<TLeaf> OffsetStrategy<TIdIndex, TStringIndex, TLeaf>::decodeLeaf(uint64_t leafValue) const {
  TLeaf* leaf = reinterpret_cast<TLeaf*>(leafValue >> 16);
  uint16_t offset = leafValue & 0xFFFF;

  return leaf->getByOffset(offset);
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
template<class TIdIndex, class TStringIndex, class TLeaf>
void OffsetStrategy<TIdIndex, TStringIndex, TLeaf>::leafCallback(TLeaf* leaf, uint16_t deltaNumber, uint16_t offset, uint64_t id, std::string value) {
  uint64_t leafValue = this->encodeLeaf(leaf, offset);

  this->reverseIndex.insert(value, leafValue);
  this->index.insert(id, leafValue);
}
#pragma GCC diagnostic pop
