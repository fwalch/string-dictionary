#ifndef H_Dictionaries
#define H_Dictionaries

namespace Dictionaries {
  static constexpr char Count = 6;
}

#include "DummyDictionary.hpp"
#include "UncompressedDictionary.hpp"
#include "SimpleDictionary.hpp"
#include "HashARTDictionary.hpp"
#include "ARTDictionary.hpp"
#include "BTreeDictionary.hpp"
#include "BPlusTreeDictionary.hpp"

std::ostream& operator<<(std::ostream &stream, const Dictionary* dict);

#endif
