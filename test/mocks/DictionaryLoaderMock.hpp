#ifndef H_DictionaryLoaderMock
#define H_DictionaryLoaderMock

#include "DictionaryLoader.hpp"
#include "gmock/gmock.h"

class DictionaryLoaderMock : public DictionaryLoader {
  public:
    DictionaryLoaderMock() : DictionaryLoader(DictionaryLoader::DictionaryType::Dummy) { }
    MOCK_METHOD1(setBasePrefix, void(std::string));
    MOCK_METHOD2(addPrefix, void(std::string, std::string));
    MOCK_METHOD2(addString, void(std::string, std::string));
};

#endif
