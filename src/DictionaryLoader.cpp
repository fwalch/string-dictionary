#include "DictionaryLoader.hpp"

using namespace std;

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
  dictionary.insert(insertValue);
}
