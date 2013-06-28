#ifndef H_DummyDictionary
#define H_DummyDictionary

#include "Dictionary.hpp"

/**
 * Dummy implementation - doesn't store data.
 */
class DummyDictionary : public Dictionary {
  public:
    ~DummyDictionary() noexcept { }

    bool bulkInsert(size_t size, std::string* values);
    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);

    const char* name() const {
      return "DummyDictionary";
    }
};

#endif
