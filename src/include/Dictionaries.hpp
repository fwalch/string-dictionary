#ifndef H_Dictionaries
#define H_Dictionaries

namespace Dictionaries {
  static constexpr char Count = 8;
}

#include "DummyDictionary.hpp"
#include "UncompressedDictionary.hpp"
#include "SimpleDictionary.hpp"
#include "HATDictionary.hpp"
#include "HashARTDictionary.hpp"
#include "ARTDictionary.hpp"
#include "ARTcDictionary.hpp"
#include "BTreeDictionary.hpp"
#include "BPlusTreeDictionary.hpp"

std::ostream& operator<<(std::ostream &stream, const Dictionary* dict);

#endif
