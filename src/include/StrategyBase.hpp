#ifndef H_StrategyBase
#define H_StrategyBase

#include <string>
#include "boost/algorithm/string.hpp"
#include <cstdint>
#include "Page.hpp"
#ifdef DEBUG
#undef NDEBUG
#include <cassert>
#endif

template<class TIdIndex, class TStringIndex, class TLeaf>
class StrategyBase {
  protected:
    TIdIndex& index;
    TStringIndex& reverseIndex;

    uint64_t encodeLeaf(TLeaf* leaf, uint16_t additionalValue) const {
      uint64_t leafValue = reinterpret_cast<uint64_t>(leaf);
      leafValue = leafValue <<16;
      leafValue |= static_cast<uint64_t>(additionalValue);

      return leafValue;
    }

    virtual PageIterator<TLeaf> decodeLeaf(uint64_t leafValue) const = 0;
    virtual void leafCallback(TLeaf* leaf, uint16_t deltaNumber, uint16_t offset, uint64_t id, std::string value) = 0;

  public:
    StrategyBase(TIdIndex& idIndex, TStringIndex& strIndex) : index(idIndex), reverseIndex(strIndex) {
    }
    virtual ~StrategyBase() { }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    virtual PageIterator<TLeaf> decodeLeaf(uint64_t leafValue, uint64_t lookupId) const {
      // We can get the offset value from the leafValue
      // and just go to the corresponding entry
#ifdef DEBUG
      auto leafIt = decodeLeaf(leafValue);
      assert(leafIt);
      if ((*leafIt).first != lookupId) {
        std::cout << "Looked for " << lookupId << ", got " << (*leafIt).first << ": " << (*leafIt).second << std::endl;
        std::cout << leafValue << std::endl;
        std::cout << (leafValue>>16 ) << std::endl;
        std::cout << (leafValue & 0xFFFF ) << std::endl;
        leafIt.debug();
      }
      assert((*leafIt).first == lookupId);
      return leafIt;
#else
      return decodeLeaf(leafValue);
#endif
    }
#pragma GCC diagnostic pop

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    virtual PageIterator<TLeaf> decodeLeaf(uint64_t leafValue, std::string lookupValue) const {
      // We can get the offset value from the leafValue
      // and just go to the corresponding entry
#ifdef DEBUG
      auto leafIt = decodeLeaf(leafValue);
      assert(leafIt);
      // Might be a range lookup, so check only prefix
      assert(boost::starts_with((*leafIt).second, lookupValue));
      return leafIt;
#else
      return decodeLeaf(leafValue);
#endif
    }
#pragma GCC diagnostic pop


    virtual bool rangeLookup(std::string prefix, PageIterator<TLeaf>& start, PageIterator<TLeaf>& end) const {
      std::pair<uint64_t, uint64_t> range = reverseIndex.rangeLookup(prefix);

      if (range.first == 0) {
        // Prefix not found
        return false;
      }
#ifdef DEBUG
      assert(range.second != 0);
#endif

      start = decodeLeaf(range.first, prefix);
      end = decodeLeaf(range.second, prefix);

#ifdef DEBUG
      assert(start);
      assert(end);
#endif

      return true;
    }
};

#endif
