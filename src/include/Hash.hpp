#ifndef H_Hash
#define H_Hash

#include <unordered_map>

template<typename TKey> class Hash {
  private:
    std::unordered_map<TKey, uint64_t> index;

  public:
    void insert(TKey key, uint64_t value);
    bool lookup(TKey key, uint64_t& value) const;
    static std::string description();
};

#endif
