#ifndef H_SimpleDictionary
#define H_SimpleDictionary

#include <string>
#include <unordered_map>
#include "Dictionary.hpp"

/**
 * Simple compression by storing each string only once.
 */
class SimpleDictionary : public Dictionary {
  public:
    /**
     * Calculates a hash value for a string pointer
     * by hashing the string value.
     */
    struct hash {
      /**
       * Calculates the hash value of the given string value.
       *
       * @param value String value to calculate hash value for.
       * @return Hash value of the given string.
       */
      size_t operator()(const char* value) const;
    };

    /**
     * Compares string pointers for equality
     * by comparing the string values.
     */
    struct equal_to {
      /**
       * Determines whether the given strings are equal.
       *
       * @param lhs Lefthand string to compare
       * @param rhs Righthand string to compare
       * @return TRUE if the strings are equal, FALSE otherwise.
       */
      bool operator()(const char* lhs, const char* rhs) const;
    };

  private:
    std::unordered_map<IdType, const char*> index;
    std::unordered_map<const char*, IdType, hash, equal_to> reverseIndex;

  public:
    ~SimpleDictionary() noexcept;
    void insert(std::string value);
};

#endif
