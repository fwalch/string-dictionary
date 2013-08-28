#ifndef H_StringDictionary
#define H_StringDictionary

#include "Dictionary.hpp"
#include "Page.hpp"
#include "LeafStore.hpp"

template<class TIndex, bool B = std::is_constructible<TIndex, LeafStore*>::value>
class ConstructHelper {
  public:
    static TIndex create(LeafStore* store);
};

template<class TIndex>
class ConstructHelper<TIndex, false> {
  public:
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
    static TIndex create(LeafStore* store) {
      return TIndex();
    }
#pragma GCC diagnostic pop
};

template<class TIndex>
class ConstructHelper<TIndex, true> {
  public:
    static TIndex create(LeafStore* store) {
      return TIndex(store);
    }
};

/**
 * Base class for dictionary implementations.
 */
template<template<typename> class TIdIndex, template<typename> class TStringIndex, class TLeaf>
class StringDictionary : public Dictionary, public LeafStore {
  private:
    TIdIndex<uint64_t> index;
    TStringIndex<std::string> reverseIndex;

    uint64_t encodeLeaf(TLeaf* leaf, uint16_t deltaNumber) const;
    PageIterator<TLeaf> decodeLeaf(uint64_t leafValue) const;
    void leafCallback(TLeaf* leaf, uint16_t deltaNumber, uint64_t id, std::string value);
    std::pair<uint64_t, std::string> getLeaf(uint64_t leafValue) const;

  public:
    StringDictionary() : index(ConstructHelper<TIdIndex<uint64_t>>::create(this)), reverseIndex(ConstructHelper<TStringIndex<std::string>>::create(this)) {
    }

    ~StringDictionary() noexcept {
    }

    std::string description() const {
      return TIdIndex<uint64_t>::description() + "/" + TStringIndex<std::string>::description() + " and " + TLeaf::description() + " as leaves";
    }

    void bulkInsert(size_t size, std::string* values);
    uint64_t insert(std::string value);
    bool lookup(std::string& value, uint64_t& id) const;
    bool lookup(uint64_t id, std::string& value) const;
};

#include "../StringDictionary.cpp"

#endif
