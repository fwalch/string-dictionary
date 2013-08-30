#ifndef H_LeafStore
#define H_LeafStore

#include <string>
#include <tuple>

/**
 * Internal interface for leaf storing.
 */
class LeafStore {
  public:
    virtual ~LeafStore();
    virtual std::string getValue(uint64_t leafValue) const = 0;
    virtual uint64_t getId(uint64_t leafValue) const = 0;
};

#endif
