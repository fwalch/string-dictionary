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

    IdType size() const;
};

#endif
