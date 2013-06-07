#ifndef H_DictionaryMock
#define H_DictionaryMock

#include "DummyDictionary.hpp"
#include "gmock/gmock.h"

class DictionaryMock : public DummyDictionary {
  public:
    MOCK_METHOD1(insert, uint64_t(std::string));
};

#endif
