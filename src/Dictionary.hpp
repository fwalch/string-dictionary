#ifndef H_Dictionary
#define H_Dictionary

#include <string>

/**
 * Base class for dictionary implementations.
 */
class Dictionary {
  protected:
    unsigned long nextId;

  public:
    typedef unsigned long IdType;
    virtual ~Dictionary();
    virtual void insert(std::string value) = 0;
};

#endif
