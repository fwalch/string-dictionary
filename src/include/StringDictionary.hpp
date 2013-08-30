#ifndef H_StringDictionary
#define H_StringDictionary

#include "Dictionary.hpp"
#include "Page.hpp"
#include "LeafStore.hpp"
#include "ConstructionStrategies.hpp"

/**
 * Helper class for different constructors
 */
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
template<template<typename> class TIdIndex, template<typename> class TStringIndex, class TLeaf, template<typename, typename, typename> class TConstructionStrategy = OffsetStrategy>
class StringDictionary : public Dictionary, public LeafStore {
#ifdef DEBUG
  public:
#else
  private:
#endif
    TIdIndex<uint64_t> index;
    TStringIndex<std::string> reverseIndex;
    TConstructionStrategy<TIdIndex<uint64_t>, TStringIndex<std::string>, TLeaf> constructionStrategy;

    inline std::string getValue(uint64_t leafValue) const {
#ifdef DEBUG
      auto it = constructionStrategy.decodeLeaf(leafValue);
      assert(it);
      return it.getValue();
#else
      return constructionStrategy.decodeLeaf(leafValue).getValue();
#endif
    }

    inline uint64_t getId(uint64_t leafValue) const {
#ifdef DEBUG
      auto it = constructionStrategy.decodeLeaf(leafValue);
      assert(it);
      return it.getId();
#else
      return constructionStrategy.decodeLeaf(leafValue).getId();
#endif
    }

  public:
    StringDictionary() : index(ConstructHelper<TIdIndex<uint64_t>>::create(this)), reverseIndex(ConstructHelper<TStringIndex<std::string>>::create(this)), constructionStrategy(TConstructionStrategy<TIdIndex<uint64_t>,  TStringIndex<std::string>, TLeaf>(index, reverseIndex)) {
      TLeaf::counter = 0;
    }

    ~StringDictionary() noexcept {
    }

    std::string description() const {
      return TLeaf::description();
      //return TIdIndex<uint64_t>::description() + "/" + TStringIndex<std::string>::description() + " and " + TLeaf::description() + " as leaves";
    }

    std::string numberOfLeaves() const {
      return std::to_string(TLeaf::counter);
    }

#ifdef DEBUG
    void debug() const {
      std::cout << "Debug dict" << std::endl;
      const_cast<StringDictionary<TIdIndex, TStringIndex, TLeaf, TConstructionStrategy>*>(this)->reverseIndex.debug();
    }
#endif

    void bulkInsert(size_t size, std::string* values);
    uint64_t insert(std::string value);
    bool lookup(std::string value, uint64_t& id) const;
    bool lookup(uint64_t id, std::string& value) const;
    void rangeLookup(std::string prefix, RangeLookupCallbackType callback) const;

    void setEx() {
      TConstructionStrategy<TIdIndex<uint64_t>, TStringIndex<std::string>, TLeaf>::throwEx = true;
    }
};

#include "../StringDictionary.cpp"

#endif
