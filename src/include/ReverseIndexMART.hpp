#ifndef H_ReverseIndexMART
#define H_ReverseIndexMART

#include <stdint.h>    // integer types
#include <IndexART.hpp>

class ReverseIndexMART {
  public:
    struct Node;
  protected:
    struct Node4;
    struct Node16;
    struct Node48;
    struct Node256;

    unsigned nextId;

    void insertNode4(Node4* node, uint8_t keyByte,Node* child);
    void insertNode16(Node16* node, uint8_t keyByte,Node* child);
    void insertNode48(Node48* node, uint8_t keyByte,Node* child);
    void insertNode256(Node256* node,uint8_t keyByte,Node* child);
    void insertNode(Node* parent,uint8_t keyByte,Node* child);
    void insertOrGrowNode4(Node4* node,Node** nodeRef,uint8_t keyByte,Node* child);
    void insertOrGrowNode16(Node16* node,Node** nodeRef,uint8_t keyByte,Node* child);
    void insertOrGrowNode48(Node48* node,Node** nodeRef,uint8_t keyByte,Node* child);
    void eraseNode4(Node4* node,Node** nodeRef,Node** leafPlace);
    void eraseNode16(Node16* node,Node** nodeRef,Node** leafPlace);
    void eraseNode48(Node48* node,Node** nodeRef,uint8_t keyByte);
    void eraseNode256(Node256* node,Node** nodeRef,uint8_t keyByte);

    void copyPrefix(Node* src,Node* dst);
    void bulkInsertRec(size_t size, std::string* values, uint32_t prefixPos, Node* node);
    Node* createRootNode(size_t size, std::string* values, uint32_t& prefixPos);
    Node* createNode(const char* prefix, uint32_t prefixLength, uint8_t numberOfChildren);

    Node* tree;
    static Node* nullNode;

    uintptr_t getLeafValue(Node* node);
    bool isLeaf(Node* node);
    Node* minimum(Node* node);
    Node* maximum(Node* node);
    bool leafMatches(Node* leaf, uint8_t key[], unsigned keyLength, unsigned depth);
    uint8_t* loadKey(uintptr_t tid);
    Node* lookupValue(Node* node, uint8_t key[], unsigned keyLength, unsigned depth);
    unsigned prefixMismatch(Node* node, uint8_t key[], unsigned depth);
    void insertValue(Node* node,Node** nodeRef,uint8_t key[],unsigned depth,uintptr_t value);
    Node** findChild(Node* n,uint8_t keyByte);
    inline Node* makeLeaf(uintptr_t tid);
    void erase(Node* node,Node** nodeRef,uint8_t key[],unsigned keyLength,unsigned depth);

  public:
    ReverseIndexMART(IndexART&);
    virtual ~ReverseIndexMART();

  private:
    uint32_t maxKeyLength;
    IndexART& index;

  public:

    void insert(std::string key, uint64_t value);
    void bulkInsert(size_t size, std::string* values);
    bool lookup(std::string key, uint64_t& value);
};

#endif
