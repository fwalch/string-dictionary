#ifndef H_DictionaryLoader
#define H_DictionaryLoader

#include <memory>
#include <string>
#include <unordered_map>
#include "Dictionary.hpp"
#include "UncompressedDictionary.hpp"
#include "DummyDictionary.hpp"
#include "SimpleDictionary.hpp"

class DictionaryLoader {
  public:
    enum DictionaryType {
      Dummy,
      Uncompressed,
      Simple
    };

  private:
    std::string basePrefix;
    std::unordered_map<std::string, std::string> prefixes;
    std::unique_ptr<Dictionary> dictionary;

  public:
    DictionaryLoader(DictionaryLoader::DictionaryType type);
    ~DictionaryLoader();

    void setBasePrefix(std::string prefix);
    void addPrefix(std::string id, std::string prefix);
    void addString(std::string prefixId, std::string value);
};

#endif
