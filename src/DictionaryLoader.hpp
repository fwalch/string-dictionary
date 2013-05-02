#ifndef H_DictionaryLoader
#define H_DictionaryLoader

#include <string>
#include <unordered_map>
#include "UncompressedDictionary.hpp"

class DictionaryLoader {
  private:
    std::string basePrefix;
    std::unordered_map<std::string, std::string> prefixes;
    UncompressedDictionary dictionary;

  public:
    void setBasePrefix(std::string prefix);
    void addPrefix(std::string id, std::string prefix);
    void addString(std::string prefixId, std::string value);
};

#endif
