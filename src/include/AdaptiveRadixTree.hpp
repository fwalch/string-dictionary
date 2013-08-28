#ifndef H_AdaptiveRadixTree
#define H_AdaptiveRadixTree

#include <cstdint>
#include "LeafStore.hpp"

class ARTBase {
  public:
    struct Node;
  protected:
    struct Node4;
    struct Node16;
    struct Node48;
    struct Node256;

    Node* tree;
    static Node* nullNode;
    LeafStore* leafStore;

    void insertNode4(Node4* node,Node** nodeRef,uint8_t keyByte,Node* child);
    void insertNode16(Node16* node,Node** nodeRef,uint8_t keyByte,Node* child);
    void insertNode48(Node48* node,Node** nodeRef,uint8_t keyByte,Node* child);
    void insertNode256(Node256* node,uint8_t keyByte,Node* child);
    void eraseNode4(Node4* node,Node** nodeRef,Node** leafPlace);
    void eraseNode16(Node16* node,Node** nodeRef,Node** leafPlace);
    void eraseNode48(Node48* node,Node** nodeRef,uint8_t keyByte);
    void eraseNode256(Node256* node,Node** nodeRef,uint8_t keyByte);

    void copyPrefix(Node* src,Node* dst) const;

    uintptr_t getLeafValue(Node* node) const;
    bool isLeaf(Node* node) const;
    Node* minimum(Node* node) const;
    Node* maximum(Node* node) const;
    bool leafMatches(Node* leaf, uint8_t key[], unsigned keyLength, unsigned depth) const;
    virtual uint8_t* loadKey(uintptr_t leafValue) const = 0;
    Node* lookupValue(Node* node, uint8_t key[], unsigned keyLength, unsigned depth) const;
    Node* lookupValuePessimistic(Node* node, uint8_t key[], unsigned keyLength, unsigned depth) const;
    unsigned prefixMismatch(Node* node, uint8_t key[], unsigned depth) const;
    void insertValue(Node* node,Node** nodeRef,uint8_t key[],unsigned depth,uintptr_t value);
    Node** findChild(Node* n,uint8_t keyByte) const;
    Node* makeLeaf(uintptr_t tid) const;
    void erase(Node* node,Node** nodeRef,uint8_t key[],unsigned keyLength,unsigned depth);

  protected:
    ARTBase(LeafStore* leafStore);
    virtual ~ARTBase();
};

template<class TKey>
class AdaptiveRadixTree : public ARTBase {
  private:
    uint8_t* loadKey(uintptr_t leafValue) const;
    uint8_t* convertKey(TKey key) const;
    unsigned keySize(TKey key) const;

  public:
    AdaptiveRadixTree(LeafStore* leafStore);
    void insert(TKey key, uintptr_t value);
    bool lookup(TKey key, uintptr_t& value) const;
};

#endif
