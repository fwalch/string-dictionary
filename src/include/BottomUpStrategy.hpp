#ifndef H_BottomUpStrategy
#define H_BottomUpStrategy

#include "StrategyBase.hpp"

template<class TIdIndex, class TStringIndex, class TLeaf>
class BottomUpStrategy : public StrategyBase<TIdIndex, TStringIndex, TLeaf> {
  public:
    BottomUpStrategy(TIdIndex& idIndex, TStringIndex& strIndex) : StrategyBase<TIdIndex, TStringIndex, TLeaf>(idIndex, strIndex) {
    }

    PageIterator<TLeaf> decodeLeaf(uint64_t leafValue) const;
    //PageIterator<TLeaf> decodeLeaf(uint64_t leafValue, uint64_t lookupId) const;
    //TODO: why!??
    PageIterator<TLeaf> decodeLeaf(uint64_t leafValue, uint64_t lookupId) const {
      return StrategyBase<TIdIndex, TStringIndex, TLeaf>::decodeLeaf(leafValue, lookupId);
    }
    PageIterator<TLeaf> decodeLeaf(uint64_t leafValue, std::string lookupValue) const;
    bool rangeLookup(std::string prefix, PageIterator<TLeaf>& start, PageIterator<TLeaf>& end) const;

    void leafCallback(TLeaf* leaf, uint16_t deltaNumber, uint16_t offset, uint64_t id, std::string value);
};

#include "../BottomUpStrategy.cpp"

#endif
