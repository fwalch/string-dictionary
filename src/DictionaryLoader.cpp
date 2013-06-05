#include "DictionaryLoader.hpp"

using namespace std;

DictionaryLoader::DictionaryLoader(DictionaryType type) {
  switch (type) {
    case DictionaryType::Dummy:
      dictionary = unique_ptr<Dictionary>(new DummyDictionary());
      break;

    case DictionaryType::Uncompressed:
      dictionary = unique_ptr<Dictionary>(new UncompressedDictionary());
      break;

    case DictionaryType::Simple:
      dictionary = unique_ptr<Dictionary>(new SimpleDictionary());
      break;

    case DictionaryType::ART:
      dictionary = unique_ptr<Dictionary>(new ARTDictionary());
      break;

    case DictionaryType::HashART:
      dictionary = unique_ptr<Dictionary>(new HashARTDictionary());
      break;
  }
}

DictionaryLoader::~DictionaryLoader() {
  dictionary.reset();
}

void DictionaryLoader::setBasePrefix(string prefix) {
  basePrefix = prefix;
}

void DictionaryLoader::addPrefix(string id, string prefix) {
  prefixes.insert(make_pair(id, prefix));
}

void DictionaryLoader::addString(string prefixId, string value) {
  string insertValue;
  if (prefixId.empty()) {
    insertValue = basePrefix + value;
  }
  else {
    insertValue = prefixes.at(prefixId) + value;
  }
  dictionary->insert(insertValue);
}
