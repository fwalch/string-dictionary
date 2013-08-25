#ifndef H_Dictionaries
#define H_Dictionaries

#include "DummyDictionary.hpp"
#include "UncompressedDictionary.hpp"
#include "SimpleDictionary.hpp"
#include "HATDictionary.hpp"
#include "HashARTDictionary.hpp"
#include "ARTDictionary.hpp"
#include "MARTDictionary.hpp"
#include "ARTcDictionary.hpp"
#include "BTreeDictionary.hpp"
#include "BPlusTreeDictionary.hpp"
#include "HARTDictionary.hpp"

std::ostream& operator<<(std::ostream &stream, const Dictionary* dict);

#endif
