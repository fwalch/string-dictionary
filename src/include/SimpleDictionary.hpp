#ifndef H_SimpleDictionary
#define H_SimpleDictionary

#include <string>
#include <unordered_map>
#include <vector>
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
    typedef std::unordered_map<uint64_t, const char*> IndexType;
    IndexType index;
    std::unordered_map<const char*, uint64_t, hash, equal_to> reverseIndex;

  public:
    ~SimpleDictionary() noexcept;

    bool bulkInsert(size_t size, std::string* values);
    uint64_t insert(std::string value);
    bool update(uint64_t& id, std::string value);
    bool lookup(std::string value, uint64_t& id);
    bool lookup(uint64_t id, std::string& value);
    Iterator rangeLookup(std::string prefix);

    std::string name() const {
      return "Hash/Hash (simple compression)";
    }

    class Iterator : public Dictionary::Iterator {
      private:
        SimpleDictionary* dict;
        std::string prefix;
        SimpleDictionary::IndexType::const_iterator iterator;

      public:
        Iterator(SimpleDictionary* dict, std::string prefix);
        const std::pair<uint64_t, std::string> operator*();
        Iterator& operator++();
        operator bool();
    };

    friend Iterator;
};

#endif
