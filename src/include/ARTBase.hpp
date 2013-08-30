#ifndef H_ARTBase
#define H_ARTBase

#include <cstdint>
#include "LeafStore.hpp"
#include <iostream>

class ARTBase {
  protected:
    // The maximum prefix length for compressed paths stored in the
    // header, if the path is longer it is loaded from the database on
    // demand
    static const unsigned maxPrefixLength=9;

    static std::string ind(uint32_t indent) {
      return std::string(2*indent, ' ');
    }
  public:
    // Shared header of all inner nodes
    struct Node {
      // length of the compressed path (prefix)
      uint32_t prefixLength;
      // number of non-null children
      uint16_t count;
      // node type
      int8_t type;
      // compressed path (prefix)
      uint8_t prefix[maxPrefixLength];

      Node(int8_t nodeType) : prefixLength(0),count(0),type(nodeType) {}
      virtual ~Node() {}

      virtual void print(uint32_t indent);

      static inline void printChild(ARTBase::Node* child, uint32_t indent) {
        if (ARTBase::isLeaf(child)) {
          std::cout << ind(indent+1) << "Leaf: " << ARTBase::getLeafValue(child) << std::endl;
        }
        else {
          child->print(indent+1);
        }
      }
    };
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

    static uintptr_t getLeafValue(Node* node);
    static bool isLeaf(Node* node);
    Node* minimum(Node* node) const;
    Node* maximum(Node* node) const;
    Node* lastChild(Node* node) const;
    virtual Node* lookupPrefix(Node* node, uint8_t prefix[], unsigned prefixLength, unsigned depth) const;
    ARTBase::Node* sartLookupPrefix(ARTBase::Node* node, uint8_t prefix[], unsigned prefixLength, unsigned depth) const;
    bool leafMatches(Node* leaf, uint8_t key[], unsigned keyLength, unsigned depth) const;
    virtual void loadKey(uintptr_t leafValue, uint8_t* key, unsigned maxKeyLength) const = 0;
    Node* lookupValue(Node* node, uint8_t key[], unsigned keyLength, unsigned depth) const;
    Node* sartLookupValue(Node* node, uint8_t key[], unsigned keyLength, unsigned depth) const;
    Node* lookupValuePessimistic(Node* node, uint8_t key[], unsigned keyLength, unsigned depth) const;
    unsigned prefixMismatch(Node* node, uint8_t key[], unsigned depth, unsigned maxKeyLength) const;
    void insertValue(Node* node,Node** nodeRef,uint8_t key[],unsigned depth,uintptr_t value, unsigned maxKeyLength);
    Node** findChild(Node* n,uint8_t keyByte) const;
    Node* secondChild(Node* n) const;
    Node** lowerThan(Node* n,uint8_t keyByte) const;
    Node** greaterThan(Node* n,uint8_t keyByte) const;
    Node* makeLeaf(uintptr_t tid) const;
    void erase(Node* node,Node** nodeRef,uint8_t key[],unsigned keyLength,unsigned depth, unsigned maxKeyLength);

  protected:
    ARTBase(LeafStore* leafStore);
    virtual ~ARTBase();
};

#endif
