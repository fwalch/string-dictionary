#ifndef H_Dictionaries
#define H_Dictionaries

namespace Dictionaries {
  static constexpr char Count = 5;
}

#include "DummyDictionary.hpp"
#include "UncompressedDictionary.hpp"
#include "SimpleDictionary.hpp"
#include "HashARTDictionary.hpp"
#include "ARTDictionary.hpp"
#include "BTreeDictionary.hpp"

std::ostream& operator<<(std::ostream &stream, const Dictionary* dict);

#endif
