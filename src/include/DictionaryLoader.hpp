#ifndef H_DictionaryLoader
#define H_DictionaryLoader

#include <memory>
#include <string>
#include <unordered_map>
#include "Dictionary.hpp"
#include "UncompressedDictionary.hpp"
#include "DummyDictionary.hpp"
#include "SimpleDictionary.hpp"
#include "ARTDictionary.hpp"
#include "HashARTDictionary.hpp"

class DictionaryLoader {
  public:
    enum DictionaryType {
      Dummy,
      Uncompressed,
      Simple,
      ART,
      HashART
    };

  private:
    std::string basePrefix;
    std::unordered_map<std::string, std::string> prefixes;
    std::unique_ptr<Dictionary> dictionary;

  public:
    DictionaryLoader(DictionaryLoader::DictionaryType type);
    virtual ~DictionaryLoader();

    virtual void setBasePrefix(std::string prefix);
    virtual void addPrefix(std::string id, std::string prefix);
    virtual void addString(std::string prefixId, std::string value);
};

#endif
