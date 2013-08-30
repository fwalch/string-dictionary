#ifndef H_RedBlack
#define H_RedBlack

#include <map>
#include <cstdint>
#include <string>

template<typename TKey> class RedBlack {
  private:
    std::map<TKey, uint64_t> index;

  public:
    void insert(TKey key, uint64_t value);
    bool lookup(TKey key, uint64_t& value) const;
    static std::string description();
};

#endif
