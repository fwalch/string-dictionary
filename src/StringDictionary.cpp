#include <cassert>
#include <type_traits>
#include <vector>
#include "StringDictionary.hpp"

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf>
uint64_t StringDictionary<TIdIndex, TStringIndex, TLeaf>::encodeLeaf(TLeaf* leaf, uint16_t deltaNumber) const {
  uint64_t leafValue = reinterpret_cast<uint64_t>(leaf);
  leafValue = leafValue <<16;
  leafValue |= static_cast<uint64_t>(deltaNumber);

  return leafValue;
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf>
PageIterator<TLeaf> StringDictionary<TIdIndex, TStringIndex, TLeaf>::decodeLeaf(uint64_t leafValue) const {
  TLeaf* leaf = reinterpret_cast<TLeaf*>(leafValue >> 16);
  uint16_t deltaNumber = leafValue & 0xFFFF;

  return leaf->get(deltaNumber);
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf>
void StringDictionary<TIdIndex, TStringIndex, TLeaf>::leafCallback(TLeaf* leaf, uint16_t deltaNumber, uint64_t id, std::string value) {
  uint64_t leafValue = encodeLeaf(leaf, deltaNumber);

  reverseIndex.insert(value, leafValue);
  index.insert(id, leafValue);
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf>
void StringDictionary<TIdIndex, TStringIndex, TLeaf>::bulkInsert(size_t size, std::string* values) {
  assert(nextId == 1);

  std::vector<std::pair<uint64_t, std::string>> insertValues;
  insertValues.reserve(size);

  for (size_t i = 0; i <size; i++) {
    insertValues.push_back(make_pair(nextId++, values[i]));
  }

  typename PageLoader<TLeaf>::CallbackType callback;
  callback = std::bind(&StringDictionary::leafCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

  TLeaf::load(insertValues, callback);
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf>
uint64_t StringDictionary<TIdIndex, TStringIndex, TLeaf>::insert(std::string value) {
  //TODO: create Leaf
  //return reverseIndex.tryInsert(value, nextId);
  throw value;
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf>
bool StringDictionary<TIdIndex, TStringIndex, TLeaf>::lookup(std::string& value, uint64_t& id) const {
  uint64_t leafValue;
  if (reverseIndex.lookup(value, leafValue)) {
    auto iterator = decodeLeaf(leafValue);
    assert(iterator);

    id = (*iterator).first;
    return true;
  }
  return false;
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf>
bool StringDictionary<TIdIndex, TStringIndex, TLeaf>::lookup(uint64_t id, std::string& value) const {
  uint64_t leafValue;
  if (index.lookup(id, leafValue)) {
    auto iterator = decodeLeaf(leafValue);
    assert(iterator);

    value = (*iterator).second;
    return true;
  }
  return false;
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf>
std::pair<uint64_t, std::string> StringDictionary<TIdIndex, TStringIndex, TLeaf>::getLeaf(uint64_t leafValue) const {
  return *decodeLeaf(leafValue);
}

template<template<typename TId> class TIdIndex, template<typename TString> class TStringIndex, class TLeaf>
void StringDictionary<TIdIndex, TStringIndex, TLeaf>::rangeLookup(std::string& prefix, RangeLookupCallbackType callback) const {
  std::pair<uint64_t, uint64_t> range = reverseIndex.rangeLookup(prefix);

  if (range.first == 0) {
    // Prefix not found
    return;
  }

  auto startIt = decodeLeaf(range.first);

  assert(range.second != 0);
  auto endValue = *decodeLeaf(range.second);

  while (startIt) {
    auto value = *startIt;
    callback(value.first, value.second);

    if (value.first == endValue.first) {
      break;
    }

    ++startIt;
  }
}
