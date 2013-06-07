#ifndef H_AdaptiveRadixTree
#define H_AdaptiveRadixTree

#include <stdint.h>    // integer types


class AdaptiveRadixTree {
  public:
    struct Node;
  protected:
    struct Node4;
    struct Node16;
    struct Node48;
    struct Node256;

    void insertNode4(Node4* node,Node** nodeRef,uint8_t keyByte,Node* child);
    void insertNode16(Node16* node,Node** nodeRef,uint8_t keyByte,Node* child);
    void insertNode48(Node48* node,Node** nodeRef,uint8_t keyByte,Node* child);
    void insertNode256(Node256* node,uint8_t keyByte,Node* child);
    void eraseNode4(Node4* node,Node** nodeRef,Node** leafPlace);
    void eraseNode16(Node16* node,Node** nodeRef,Node** leafPlace);
    void eraseNode48(Node48* node,Node** nodeRef,uint8_t keyByte);
    void eraseNode256(Node256* node,Node** nodeRef,uint8_t keyByte);

    void copyPrefix(Node* src,Node* dst);

    Node* tree;
    static Node* nullNode;

    uintptr_t getLeafValue(Node* node);
    bool isLeaf(Node* node);
    Node* minimum(Node* node);
    Node* maximum(Node* node);
    bool leafMatches(Node* leaf, uint8_t key[], unsigned keyLength, unsigned depth);
    virtual uint8_t* loadKey(uintptr_t tid) = 0;
    Node* lookupValue(Node* node, uint8_t key[], unsigned keyLength, unsigned depth);
    Node* lookupValuePessimistic(Node* node, uint8_t key[], unsigned keyLength, unsigned depth);
    unsigned prefixMismatch(Node* node, uint8_t key[], unsigned depth);
    void insertValue(Node* node,Node** nodeRef,uint8_t key[],unsigned depth,uintptr_t value);
    Node** findChild(Node* n,uint8_t keyByte);
    inline Node* makeLeaf(uintptr_t tid);
    void erase(Node* node,Node** nodeRef,uint8_t key[],unsigned keyLength,unsigned depth);

  public:
    AdaptiveRadixTree();
    virtual ~AdaptiveRadixTree();
};

#endif
