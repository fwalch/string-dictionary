#ifndef H_Dictionary
#define H_Dictionary

#include <string>

/**
 * Base class for dictionary implementations.
 */
class Dictionary {
  protected:
    unsigned long nextId = 0;

  public:
    typedef unsigned long IdType;
    virtual ~Dictionary() noexcept;
    virtual void insert(std::string value) = 0;
};

#endif
