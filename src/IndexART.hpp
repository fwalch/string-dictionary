#ifndef H_IndexART
#define H_IndexART

#include <string>
#include <vector>
#include "AdaptiveRadixTree.hpp"

class IAReverseIndexART;

class IndexART : public AdaptiveRadixTree {
  private:
    std::vector<std::string> values;

  protected:
    void loadKey(uintptr_t tid, uint8_t key[]);

  public:
    IndexART();
    void insert(uint64_t key, std::string value);
    bool lookup(uint64_t key, std::string& value);

  friend class IAReverseIndexART;
};

#endif
