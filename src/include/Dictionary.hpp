#ifndef H_Dictionary
#define H_Dictionary

#include <string>
#include <functional>

/**
 * Base class for dictionary implementations.
 */
class Dictionary {
  protected:
    uint64_t nextId = 1;

  public:
    virtual ~Dictionary() noexcept;

    /**
     * Callback type for range (prefix) lookups
     */
    typedef std::function<void(uint64_t, std::string)> RangeLookupCallbackType;

    /**
     * Inserts multiple string values into the dictionary.
     *
     * @param size Number of string values to insert
     * @param values Pointer to an array of string values to insert
     * @return True if insertion successful, false otherwise.
     */
    //TODO: change to iterators
    virtual void bulkInsert(size_t size, std::string* values) = 0;

    /**
     * Inserts a single string value into the dictionary.
     *
     * @param value String value to insert
     * @return ID assigned to the inserted value
     */
    virtual uint64_t insert(std::string value) = 0;

    /**
     * Looks up a string by its value, giving its ID.
     *
     * @param [in] value Value to look up
     * @param [out] id ID of the given value
     * @return True if the given value was found, false otherwise
     */
    virtual bool lookup(std::string& value, uint64_t& id) const = 0;

    /**
     * Looks up a string by its ID, giving its value.
     *
     * @param [in] id ID to look up
     * @param [out] value Value of the given ID
     * @return True if the given ID was found, false otherwise
     */
    virtual bool lookup(uint64_t id, std::string& value) const = 0;

    /**
     * Looks up a all values starting with a given prefix.
     *
     * @param prefix Prefix value
     * @return Iterator for all matching values
     */
    virtual void rangeLookup(std::string& prefix, RangeLookupCallbackType callback) const = 0;

    /**
     * Returns the number of unique string values in the dictionary.
     * @return Number of unique string values in the dictionary
     */
    uint64_t size() const;

    /**
     * Returns a human-readable description of the specific dictionary implementation.
     * @returns Human-readable description of the dictionary
     */
    virtual std::string description() const = 0;
};

std::ostream& operator<<(std::ostream &stream, const Dictionary* dict);

#endif
