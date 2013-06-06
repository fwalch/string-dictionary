#ifndef H_Dictionary
#define H_Dictionary

#include <string>

/**
 * Base class for dictionary implementations.
 */
class Dictionary {
  public:
    typedef unsigned long IdType;

  protected:
    IdType nextId = 0;

  public:
    virtual ~Dictionary() noexcept;
    virtual void insert(std::string value) = 0;
    /*virtual void remove(IdType id) = 0;
    virtual void bulkInsert(std::string* values, size_t numberOfValues) = 0;
    virtual bool lookup(std::string value, IdType& id) = 0;
    virtual bool lookup(IdType id, std::string& value) = 0;*/

    IdType size() const;
    virtual const char* name() const = 0;
};

#endif
