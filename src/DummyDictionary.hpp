#ifndef H_DummyDictionary
#define H_DummyDictionary

#include "Dictionary.hpp"

/**
 * Dummy implementation - doesn't store data.
 */
class DummyDictionary : public Dictionary {
  public:
    ~DummyDictionary() { }
    void insert(std::string value);
};

#endif
