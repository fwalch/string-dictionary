#ifndef H_Dictionary
#define H_Dictionary

#include <string>

/**
 * Base class for dictionary implementations.
 */
class Dictionary {
  protected:
    uint64_t nextId = 0;

  public:
    virtual ~Dictionary() noexcept;

    virtual bool bulkInsert(size_t size, std::string* values) = 0;
    virtual uint64_t insert(std::string value) = 0;
    virtual bool update(uint64_t& id, std::string value) = 0;
    virtual bool lookup(std::string value, uint64_t& id) = 0;
    virtual bool lookup(uint64_t id, std::string& value) = 0;

    uint64_t size() const;
    virtual const char* name() const = 0;
};

#endif
