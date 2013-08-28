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
    virtual std::pair<uint64_t, std::string> getLeaf(uint64_t leafValue) const = 0;
};

#endif
