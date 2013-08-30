#ifdef DEBUG
#undef NDEBUG
#include <cassert>
#endif
#include <type_traits>
#include <vector>
#include "StringDictionary.hpp"

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf, template<typename, typename, typename> class TConstructionStrategy>
void StringDictionary<TIdIndex, TStringIndex, TLeaf, TConstructionStrategy>::bulkInsert(size_t size, std::string* values) {
#ifdef DEBUG
  assert(nextId == 1);
#endif

  std::vector<std::pair<uint64_t, std::string>> insertValues;
  insertValues.reserve(size);

  for (size_t i = 0; i <size; i++) {
    insertValues.push_back(make_pair(nextId++, values[i]));
  }

  typename PageLoader<TLeaf>::CallbackType callback;
  callback = std::bind(&TConstructionStrategy<TIdIndex<uint64_t>, TStringIndex<std::string>, TLeaf>::leafCallback, constructionStrategy, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4, std::placeholders::_5);

  TLeaf::load(insertValues, callback);
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf, template<typename, typename, typename> class TConstructionStrategy>
uint64_t StringDictionary<TIdIndex, TStringIndex, TLeaf, TConstructionStrategy>::insert(std::string value) {
  //TODO: create Leaf
  //return reverseIndex.tryInsert(value, nextId);
  throw value;
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf, template<typename, typename, typename> class TConstructionStrategy>
bool StringDictionary<TIdIndex, TStringIndex, TLeaf, TConstructionStrategy>::lookup(std::string value, uint64_t& id) const {
  uint64_t leafValue;
  if (reverseIndex.lookup(value, leafValue)) {
    auto iterator = constructionStrategy.decodeLeaf(leafValue, value);

#ifdef DEBUG
    if (!iterator) {
      std::cout << "Iterator not found for " << value << "; debug." << std::endl;
      debug();
      std::cout << "Second lookup" << std::endl;
      reverseIndex.lookup(value, leafValue);
      std::cout << "---" << std::endl;
      std::cout << "Leaf value: " << leafValue << std::endl;
      std::cout << "---" << std::endl;
      iterator.debug();
      throw Exception("Iterator not found for "+value+"; debug.");
    }
    //assert(iterator);
#endif

#ifdef DEBUG
    auto itValue = *iterator;
    id = itValue.first;
    if (value != itValue.second) {
      //debug();
      std::cout << "---" << std::endl;
      std::cout << "Leaf value: " << leafValue << std::endl;
      std::cout << "Actual value: " << value << std::endl;
      std::cout << "Found value: " << itValue.second << std::endl;
      std::cout << "---" << std::endl;
      iterator.debug();
      throw Exception("Iterator value"+itValue.second+" doesn't match "+value+"; debug.");
    }
    assert(value == itValue.second);
#else
    id = iterator.getId();
#endif

    return true;
  }
  std::cout << "Reverse not found" << std::endl;
  return false;
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf, template<typename, typename, typename> class TConstructionStrategy>
bool StringDictionary<TIdIndex, TStringIndex, TLeaf, TConstructionStrategy>::lookup(uint64_t id, std::string& value) const {
  uint64_t leafValue;
  if (index.lookup(id, leafValue)) {
    auto iterator = constructionStrategy.decodeLeaf(leafValue, id);

#ifdef DEBUG
    assert(iterator);
#endif

#ifdef DEBUG
    auto itValue = *iterator;
    value = itValue.second;
    assert(id == itValue.first);
#else
    value = iterator.getValue();
#endif

    return true;
  }
  return false;
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf, template<typename, typename, typename> class TConstructionStrategy>
void StringDictionary<TIdIndex, TStringIndex, TLeaf, TConstructionStrategy>::rangeLookup(std::string prefix, RangeLookupCallbackType callback) const {
  PageIterator<TLeaf> startIt, endIt;
  if (constructionStrategy.rangeLookup(prefix, startIt, endIt)) {

    uint64_t endId = (*endIt).first;
    while (startIt) {
      auto value = *startIt;
      callback(value.first, value.second);

      if (value.first == endId) {
        break;
      }

      ++startIt;
    }
  }
}
