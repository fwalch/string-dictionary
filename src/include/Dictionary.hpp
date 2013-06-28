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

    /**
     * Inserts multiple string values into the dictionary.
     *
     * @param size Number of string values to insert
     * @param values Pointer to an array of string values to insert
     * @return True if insertion successful, false otherwise.
     */
    virtual bool bulkInsert(size_t size, std::string* values) = 0;

    /**
     * Inserts a single string value into the dictionary.
     *
     * @param value String value to insert
     * @return ID assigned to the inserted value
     */
    virtual uint64_t insert(std::string value) = 0;

    /**
     * Updates a string, identified by an ID, to a new value.
     *
     * The id parameter will be updated to the new value's ID.
     * @param id The string's ID
     * @param value The new value to assign
     * @return True if update successful, false otherwise
     */
    virtual bool update(uint64_t& id, std::string value) = 0;

    /**
     * Looks up a string by its value, giving its ID.
     *
     * @param [in] value Value to look up
     * @param [out] id ID of the given value
     * @return True if the given value was found, false otherwise
     */
    virtual bool lookup(std::string value, uint64_t& id) = 0;

    /**
     * Looks up a string by its ID, giving its value.
     *
     * @param [in] id ID to look up
     * @param [out] value Value of the given ID
     * @return True if the given ID was found, false otherwise
     */
    virtual bool lookup(uint64_t id, std::string& value) = 0;

    /**
     * Returns the number of unique string values in the dictionary.
     * @return Number of unique string values in the dictionary
     */
    uint64_t size() const;

    /**
     * Returns a human-readable name for the specific dictionary implementation.
     * @returns Human-readable name of the dictionary
     */
    virtual const char* name() const = 0;
};

#endif
