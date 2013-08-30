#include "ARTBase.hpp"

#include <cstdlib>    // malloc, free
#include <cstring>    // memset, memcpy
#include <emmintrin.h> // x86 SSE intrinsics
#undef NDEBUG
#include <cassert>

/**
 * @file
 * Adapted from ART.cpp, http://www-db.in.tum.de/~leis/
 *
 * > Adaptive Radix Tree
 * > Viktor Leis, 2012
 * > leis@in.tum.de
 *
 */

using namespace std;

ARTBase::ARTBase(LeafStore* store) : tree(NULL), leafStore(store) {
}

ARTBase::~ARTBase() {
}

// Constants for the node types
static const int8_t NodeType4=0;
static const int8_t NodeType16=1;
static const int8_t NodeType48=2;
static const int8_t NodeType256=3;

inline unsigned min(unsigned a,unsigned b);
unsigned min(unsigned a,unsigned b) {
  // Helper function
  return (a<b)?a:b;
}

inline uint8_t flipSign(uint8_t keyByte);
uint8_t flipSign(uint8_t keyByte) {
  // Flip the sign bit, enables signed SSE comparison of unsigned values, used by Node16
  return keyByte^128;
}


void ARTBase::Node::print(uint32_t indent) {
  cout << ind(indent) << "Children: " << count << endl;
  cout << ind(indent) << "Prefix: " << string(reinterpret_cast<char*>(prefix), min(prefixLength, 9 /* ARTBase::maxPrefixLength */)) << endl;
}

// Node with up to 4 children
struct ARTBase::Node4 : Node {
  uint8_t key[4];
  Node* child[4];

  Node4() : Node(NodeType4) {
    memset(key,0,sizeof(key));
    memset(child,0,sizeof(child));
  }

  void print(uint32_t indent);
};

void ARTBase::Node4::print(uint32_t indent) {
  Node::print(indent);

  for (size_t i = 0; i < count; i++) {
    cout << ind(indent) << "Key byte: " << (char)key[i] << endl;
    printChild(child[i], indent);
  }
}

// Node with up to 16 children
struct ARTBase::Node16 : Node {
  uint8_t key[16];
  Node* child[16];

  Node16() : Node(NodeType16) {
    memset(key,0,sizeof(key));
    memset(child,0,sizeof(child));
  }

  void print(uint32_t indent);
};

void ARTBase::Node16::print(uint32_t indent) {
  Node::print(indent);

  for (size_t i = 0; i < count; i++) {
    cout << ind(indent) << "Key byte: " << (char)flipSign(key[i]) << endl;
    printChild(child[i], indent);
  }
}

static const uint8_t emptyMarker=48;

// Node with up to 48 childreconst n
struct ARTBase::Node48 : Node {
  uint8_t childIndex[256];
  Node* child[48];

  Node48() : Node(NodeType48) {
    memset(childIndex,emptyMarker,sizeof(childIndex));
    memset(child,0,sizeof(child));
  }

  void print(uint32_t indent);
};

void ARTBase::Node48::print(uint32_t indent) {
  Node::print(indent);

  for (uint16_t i = 0; i < 256; i++) {
    if (childIndex[i] == emptyMarker) continue;
    cout << ind(indent) << "Key byte: " << (char)i << endl;

    printChild(child[childIndex[i]], indent);
  }
}

// Node with up to 256 children
struct ARTBase::Node256 : Node {
  Node* child[256];

  Node256() : Node(NodeType256) {
    memset(child,0,sizeof(child));
  }

  void print(uint32_t indent);
};

void ARTBase::Node256::print(uint32_t indent) {
  Node::print(indent);

  for (uint16_t i = 0; i < 256; i++) {
    if (!child[i]) continue;
    cout << ind(indent) << "Key byte: " << (char)i << endl;
    printChild(child[i], indent);
  }
}

inline ARTBase::Node* ARTBase::makeLeaf(uintptr_t tid) const {
  // Create a pseudo-leaf
  return reinterpret_cast<Node*>((tid<<1)|1);
}

uintptr_t ARTBase::getLeafValue(Node* node) {
  // The the value stored in the pseudo-leaf
  return reinterpret_cast<uintptr_t>(node)>>1;
}

bool ARTBase::isLeaf(Node* node) {
  // Is the node a leaf?
  return reinterpret_cast<uintptr_t>(node)&1;
}

// This address is used to communicate that search failed
ARTBase::Node* ARTBase::nullNode=NULL;

static inline unsigned ctz(uint16_t x) {
  // Count trailing zeros, only defined for x>0
#ifdef __GNUC__
  return static_cast<unsigned>(__builtin_ctz(x));
#else
  // Adapted from Hacker's Delight
  unsigned n=1;
  if ((x&0xFF)==0) {n+=8; x=x>>8;}
  if ((x&0x0F)==0) {n+=4; x=x>>4;}
  if ((x&0x03)==0) {n+=2; x=x>>2;}
  return n-(x&1);
#endif
}

ARTBase::Node** ARTBase::findChild(Node* n,uint8_t keyByte) const {
  // Find the next child for the keyByte
  switch (n->type) {
    case NodeType4: {
                      Node4* node=static_cast<Node4*>(n);
                      for (unsigned i=0;i<node->count;i++)
                        if (node->key[i]==keyByte)
                          return &node->child[i];
                      return &nullNode;
                    }
    case NodeType16: {
                       Node16* node=static_cast<Node16*>(n);
                       __m128i cmp=_mm_cmpeq_epi8(_mm_set1_epi8(static_cast<char>(flipSign(keyByte))),_mm_loadu_si128(reinterpret_cast<__m128i*>(node->key)));
                       unsigned bitfield=_mm_movemask_epi8(cmp)&((1<<node->count)-1);
                       if (bitfield)
                         return &node->child[ctz(static_cast<uint16_t>(bitfield))]; else
                           return &nullNode;
                     }
    case NodeType48: {
                       Node48* node=static_cast<Node48*>(n);
                       if (node->childIndex[keyByte]!=emptyMarker)
                         return &node->child[node->childIndex[keyByte]]; else
                           return &nullNode;
                     }
    case NodeType256: {
                        Node256* node=static_cast<Node256*>(n);
                        return &(node->child[keyByte]);
                      }
  }
  throw; // Unreachable
}

ARTBase::Node* ARTBase::minimum(ARTBase::Node* node) const {
  // Find the leaf with smallest key
  if (!node)
    return NULL;

  if (isLeaf(node))
    return node;

  switch (node->type) {
    case NodeType4: {
                      Node4* n=static_cast<Node4*>(node);
                      return minimum(n->child[0]);
                    }
    case NodeType16: {
                       Node16* n=static_cast<Node16*>(node);
                       return minimum(n->child[0]);
                     }
    case NodeType48: {
                       Node48* n=static_cast<Node48*>(node);
                       unsigned pos=0;
                       while (n->childIndex[pos]==emptyMarker)
                         pos++;
                       return minimum(n->child[n->childIndex[pos]]);
                     }
    case NodeType256: {
                        Node256* n=static_cast<Node256*>(node);
                        unsigned pos=0;
                        while (!n->child[pos])
                          pos++;
                        return minimum(n->child[pos]);
                      }
  }
  throw; // Unreachable
}

ARTBase::Node* ARTBase::lastChild(ARTBase::Node* node) const {
  // Find the leaf with largest key
  if (!node)
    return NULL;

  if (isLeaf(node))
    return node;

  switch (node->type) {
    case NodeType4: {
                      Node4* n=static_cast<Node4*>(node);
                      return (n->child[n->count-1]);
                    }
    case NodeType16: {
                       Node16* n=static_cast<Node16*>(node);
                       return (n->child[n->count-1]);
                     }
    case NodeType48: {
                       Node48* n=static_cast<Node48*>(node);
                       unsigned pos=255;
                       while (n->childIndex[pos]==emptyMarker)
                         pos--;
                       return (n->child[n->childIndex[pos]]);
                     }
    case NodeType256: {
                        Node256* n=static_cast<Node256*>(node);
                        unsigned pos=255;
                        while (!n->child[pos])
                          pos--;
                        return (n->child[pos]);
                      }
  }
  throw; // Unreachable
}

ARTBase::Node* ARTBase::maximum(ARTBase::Node* node) const {
  // Find the leaf with largest key
  if (!node)
    return NULL;

  if (isLeaf(node))
    return node;

  switch (node->type) {
    case NodeType4: {
                      Node4* n=static_cast<Node4*>(node);
                      return maximum(n->child[n->count-1]);
                    }
    case NodeType16: {
                       Node16* n=static_cast<Node16*>(node);
                       return maximum(n->child[n->count-1]);
                     }
    case NodeType48: {
                       Node48* n=static_cast<Node48*>(node);
                       unsigned pos=255;
                       while (n->childIndex[pos]==emptyMarker)
                         pos--;
                       return maximum(n->child[n->childIndex[pos]]);
                     }
    case NodeType256: {
                        Node256* n=static_cast<Node256*>(node);
                        unsigned pos=255;
                        while (!n->child[pos])
                          pos--;
                        return maximum(n->child[pos]);
                      }
  }
  throw; // Unreachable
}

bool ARTBase::leafMatches(Node* leaf,uint8_t key[],unsigned keyLength,unsigned depth) const {
  // Check if the key of the leaf is equal to the searched key
  if (depth!=keyLength) {
    uint8_t leafKey[keyLength];
    loadKey(getLeafValue(leaf), leafKey, keyLength);
    for (unsigned i=depth;i<keyLength;i++)
      if (leafKey[i]!=key[i])
        return false;
  }
  return true;
}

unsigned ARTBase::prefixMismatch(Node* node,uint8_t key[],unsigned depth, unsigned keyLength) const {
  // Compare the key with the prefix of the node, return the number matching bytes
  unsigned pos;
  if (node->prefixLength>maxPrefixLength) {
    for (pos=0;pos<maxPrefixLength;pos++)
      if (key[depth+pos]!=node->prefix[pos])
        return pos;
    uint8_t minKey[keyLength];
    loadKey(getLeafValue(minimum(node)), minKey, keyLength);
    for (;pos<node->prefixLength;pos++)
      if (key[depth+pos]!=minKey[depth+pos])
        return pos;
  } else {
    for (pos=0;pos<node->prefixLength;pos++)
      if (key[depth+pos]!=node->prefix[pos])
        return pos;
  }
  return pos;
}

ARTBase::Node* ARTBase::lookupValue(ARTBase::Node* node,uint8_t key[],unsigned keyLength,unsigned depth) const {
  // Find the node with a matching key, optimistic version

  bool skippedPrefix=false; // Did we optimistically skip some prefix without checking it?

  while (node!=NULL) {
    if (isLeaf(node)) {
      if (!skippedPrefix&&depth==keyLength) // No check required
        return node;

      if (depth!=keyLength) {
        // Check leaf
        uint8_t leafKey[keyLength];
        loadKey(getLeafValue(node), leafKey, keyLength);
        for (unsigned i=(skippedPrefix?0:depth);i<keyLength;i++)
          if (leafKey[i]!=key[i])
            return NULL;
      }
      return node;
    }

    if (node->prefixLength) {
      if (node->prefixLength<maxPrefixLength) {
        for (unsigned pos=0;pos<node->prefixLength;pos++)
          if (key[depth+pos]!=node->prefix[pos])
            return NULL;
      } else
        skippedPrefix=true;
      depth+=node->prefixLength;
    }

    node=*findChild(node,key[depth]);
    depth++;
  }

  return NULL;
}

ARTBase::Node* ARTBase::lookupPrefix(ARTBase::Node* node,uint8_t key[],unsigned keyLength,unsigned depth) const {
  // Find the node with a matching prefix

  while (node!=NULL) {
    if (isLeaf(node)) {
      if (keyLength == depth) {
        return node;
      }
      return NULL;
    }

    unsigned matchingChars = prefixMismatch(node, key, depth, keyLength);
    if (matchingChars == keyLength) {
      // Return inner node
      return node;
    }
    else if (matchingChars != node->prefixLength) {
      return NULL;
    }

    depth+=node->prefixLength;

    node=*findChild(node,key[depth]);
    depth++;
  }

  return NULL;
}

ARTBase::Node* ARTBase::secondChild(ARTBase::Node* node) const {
  assert(node->count > 1);
  Node* child;
  switch (node->type) {
    case NodeType4: {
                      Node4* n = static_cast<Node4*>(node);
                      child = n->child[n->count-2];
                      break;
                    }
    case NodeType16:
                    {
                      Node16* n = static_cast<Node16*>(node);
                      child = n->child[n->count-2];
                      break;
                    }
    case NodeType48:
                    {
                      Node48* n = static_cast<Node48*>(node);
                      unsigned i = 47;
                      while (n->childIndex[i]==emptyMarker) i--;
                      i--;
                      while (n->childIndex[i]==emptyMarker) i--;
                      child = n->child[n->childIndex[i]];
                      break;
                    }
    case NodeType256:
                    {
                      Node256* n = static_cast<Node256*>(node);
                      unsigned i = 255;
                      while (n->child[i] == NULL) i--;
                      i--;
                      while (n->child[i] == NULL) i--;
                      child = n->child[i];
                      break;
                    }
    default: throw;
  }
  assert(child != NULL);
  return maximum(child);
}

ARTBase::Node* ARTBase::sartLookupValue(ARTBase::Node* node,uint8_t key[],unsigned keyLength,unsigned depth) const {
  //std::cout << "lookup" << std::endl;

  while (node != NULL) {
    if (isLeaf(node)) {
      return node;
    }

    unsigned mismatchPos = prefixMismatch(node, key, depth, keyLength);
    if (mismatchPos != node->prefixLength) {
      return minimum(node);
    }

    if (keyLength == depth+node->prefixLength) {
      return minimum(node);
    }

    depth += node->prefixLength;

    Node** child = findChild(node, key[depth]);
    if (child != NULL && *child) {
      node = *child;
      depth++;
      continue;
    }

    assert(node != NULL);
    child = greaterThan(node, key[depth]);
    if (child == NULL || !(*child)) {
      return maximum(*lowerThan(node, key[depth]));
    }
    return minimum(*child);
  }

  return NULL;
}

ARTBase::Node* ARTBase::lookupValuePessimistic(ARTBase::Node* node,uint8_t key[],unsigned keyLength,unsigned depth) const {
  // Find the node with a matching key, alternative pessimistic version

  while (node!=NULL) {
    if (isLeaf(node)) {
      if (leafMatches(node,key,keyLength,depth))
        return node;
      return NULL;
    }

    if (prefixMismatch(node,key,depth,keyLength)!=node->prefixLength)
      return NULL; else
        depth+=node->prefixLength;

    node=*findChild(node,key[depth]);
    depth++;
  }

  return NULL;
}

void ARTBase::copyPrefix(ARTBase::Node* src,ARTBase::Node* dst) const {
  // Helper function that copies the prefix from the source to the destination node
  dst->prefixLength=src->prefixLength;
  memcpy(dst->prefix,src->prefix,min(src->prefixLength,maxPrefixLength));
}

void ARTBase::insertValue(Node* node,Node** nodeRef,uint8_t key[],unsigned depth,uintptr_t value, unsigned maxKeyLength) {
  // Insert the leaf value into the tree

  if (node==NULL) {
    *nodeRef=makeLeaf(value);
    return;
  }

  if (isLeaf(node)) {
    // Replace leaf with Node4 and store both leaves in it
    uint8_t existingKey[maxKeyLength];
    loadKey(getLeafValue(node), existingKey, maxKeyLength);
#ifdef DEBUG
    assert(existingKey != nullptr);
#endif
    unsigned newPrefixLength=0;
    while (true) {
      if (existingKey[depth+newPrefixLength]!=key[depth+newPrefixLength]) break;
      newPrefixLength++;
    }

    Node4* newNode=new Node4();
    newNode->prefixLength=newPrefixLength;
    memcpy(newNode->prefix,key+depth,min(newPrefixLength,maxPrefixLength));
    *nodeRef=newNode;

    insertNode4(newNode,nodeRef,existingKey[depth+newPrefixLength],node);
    insertNode4(newNode,nodeRef,key[depth+newPrefixLength],makeLeaf(value));
    return;
  }

  // Handle prefix of inner node
  if (node->prefixLength) {
    unsigned mismatchPos=prefixMismatch(node,key,depth,maxKeyLength);
    if (mismatchPos!=node->prefixLength) {
      // Prefix differs, create new node
      Node4* newNode=new Node4();
      *nodeRef=newNode;
      newNode->prefixLength=mismatchPos;
      memcpy(newNode->prefix,node->prefix,min(mismatchPos,maxPrefixLength));
      // Break up prefix
      if (node->prefixLength<maxPrefixLength) {
        insertNode4(newNode,nodeRef,node->prefix[mismatchPos],node);
        node->prefixLength-=(mismatchPos+1);
        memmove(node->prefix,node->prefix+mismatchPos+1,min(node->prefixLength,maxPrefixLength));
      } else {
        node->prefixLength-=(mismatchPos+1);
        uint8_t minKey[maxKeyLength];
        loadKey(getLeafValue(minimum(node)), minKey, maxKeyLength);
        insertNode4(newNode,nodeRef,minKey[depth+mismatchPos],node);
        memmove(node->prefix,minKey+depth+mismatchPos+1,min(node->prefixLength,maxPrefixLength));
      }
      insertNode4(newNode,nodeRef,key[depth+mismatchPos],makeLeaf(value));
      return;
    }
    depth+=node->prefixLength;
  }

  // Recurse
  Node** child=findChild(node,key[depth]);
  if (*child) {
    insertValue(*child,child,key,depth+1,value,maxKeyLength);
    return;
  }

  // Insert leaf into inner node
  Node* newNode=makeLeaf(value);
  switch (node->type) {
    case NodeType4: insertNode4(static_cast<Node4*>(node),nodeRef,key[depth],newNode); break;
    case NodeType16: insertNode16(static_cast<Node16*>(node),nodeRef,key[depth],newNode); break;
    case NodeType48: insertNode48(static_cast<Node48*>(node),nodeRef,key[depth],newNode); break;
    case NodeType256: insertNode256(static_cast<Node256*>(node),key[depth],newNode); break;
  }
}

void ARTBase::insertNode4(Node4* node,Node** nodeRef,uint8_t keyByte,Node* child) {
  // Insert leaf into inner node
  if (node->count<4) {
    // Insert element
    unsigned pos;
    for (pos=0;(pos<node->count)&&(node->key[pos]<keyByte);pos++);
    memmove(node->key+pos+1,node->key+pos,node->count-pos);
    memmove(node->child+pos+1,node->child+pos,(node->count-pos)*sizeof(uintptr_t));
    node->key[pos]=keyByte;
    node->child[pos]=child;
    node->count++;
  } else {
    // Grow to Node16
    Node16* newNode=new Node16();
    *nodeRef=newNode;
    newNode->count=4;
    copyPrefix(node,newNode);
    for (unsigned i=0;i<4;i++)
      newNode->key[i]=flipSign(node->key[i]);
    memcpy(newNode->child,node->child,node->count*sizeof(uintptr_t));
    delete node;
    return insertNode16(newNode,nodeRef,keyByte,child);
  }
}

void ARTBase::insertNode16(Node16* node,Node** nodeRef,uint8_t keyByte,Node* child) {
  // Insert leaf into inner node
  if (node->count<16) {
    // Insert element
    uint8_t keyByteFlipped=flipSign(keyByte);
    __m128i cmp=_mm_cmplt_epi8(_mm_set1_epi8(static_cast<char>(keyByteFlipped)),_mm_loadu_si128(reinterpret_cast<__m128i*>(node->key)));
    uint16_t bitfield=_mm_movemask_epi8(cmp)&(0xFFFF>>(16-node->count));
    unsigned pos=bitfield?ctz(bitfield):node->count;
    memmove(node->key+pos+1,node->key+pos,node->count-pos);
    memmove(node->child+pos+1,node->child+pos,(node->count-pos)*sizeof(uintptr_t));
    node->key[pos]=keyByteFlipped;
    node->child[pos]=child;
    node->count++;
  } else {
    // Grow to Node48
    Node48* newNode=new Node48();
    *nodeRef=newNode;
    memcpy(newNode->child,node->child,node->count*sizeof(uintptr_t));
    for (uint8_t i=0;i<node->count;i++)
      newNode->childIndex[flipSign(node->key[i])]=i;
    copyPrefix(node,newNode);
    newNode->count=node->count;
    delete node;
    return insertNode48(newNode,nodeRef,keyByte,child);
  }
}

void ARTBase::insertNode48(Node48* node,Node** nodeRef,uint8_t keyByte,Node* child) {
  // Insert leaf into inner node
  if (node->count<48) {
    // Insert element
    unsigned pos=node->count;
    if (node->child[pos])
      for (pos=0;node->child[pos]!=NULL;pos++);
    node->child[pos]=child;
    node->childIndex[keyByte]=static_cast<uint8_t>(pos);
    node->count++;
  } else {
    // Grow to Node256
    Node256* newNode=new Node256();
    for (unsigned i=0;i<256;i++)
      if (node->childIndex[i]!=48)
        newNode->child[i]=node->child[node->childIndex[i]];
    newNode->count=node->count;
    copyPrefix(node,newNode);
    *nodeRef=newNode;
    delete node;
    return insertNode256(newNode,keyByte,child);
  }
}

void ARTBase::insertNode256(Node256* node,uint8_t keyByte,Node* child) {
  // Insert leaf into inner node
  node->count++;
  node->child[keyByte]=child;
}

void ARTBase::erase(Node* node,Node** nodeRef,uint8_t key[],unsigned keyLength,unsigned depth, unsigned maxKeyLength) {
  // Delete a leaf from a tree

  if (!node)
    return;

  if (isLeaf(node)) {
    // Make sure we have the right leaf
    if (leafMatches(node,key,keyLength,depth))
      *nodeRef=NULL;
    return;
  }

  // Handle prefix
  if (node->prefixLength) {
    if (prefixMismatch(node,key,depth,maxKeyLength)!=node->prefixLength)
      return;
    depth+=node->prefixLength;
  }

  Node** child=findChild(node,key[depth]);
  if (isLeaf(*child)&&leafMatches(*child,key,keyLength,depth)) {
    // Leaf found, delete it in inner node
    switch (node->type) {
      case NodeType4: eraseNode4(static_cast<Node4*>(node),nodeRef,child); break;
      case NodeType16: eraseNode16(static_cast<Node16*>(node),nodeRef,child); break;
      case NodeType48: eraseNode48(static_cast<Node48*>(node),nodeRef,key[depth]); break;
      case NodeType256: eraseNode256(static_cast<Node256*>(node),nodeRef,key[depth]); break;
    }
  } else {
    //Recurse
    erase(*child,child,key,keyLength,depth+1,maxKeyLength);
  }
}

void ARTBase::eraseNode4(Node4* node,Node** nodeRef,Node** leafPlace) {
  // Delete leaf from inner node
  unsigned long pos=static_cast<unsigned long>(leafPlace-node->child);
  memmove(node->key+pos,node->key+pos+1,node->count-pos-1);
  memmove(node->child+pos,node->child+pos+1,(node->count-pos-1)*sizeof(uintptr_t));
  node->count--;

  if (node->count==1) {
    // Get rid of one-way node
    Node* child=node->child[0];
    if (!isLeaf(child)) {
      // Concantenate prefixes
      unsigned l1=node->prefixLength;
      if (l1<maxPrefixLength) {
        node->prefix[l1]=node->key[0];
        l1++;
      }
      if (l1<maxPrefixLength) {
        unsigned l2=min(child->prefixLength,maxPrefixLength-l1);
        memcpy(node->prefix+l1,child->prefix,l2);
        l1+=l2;
      }
      // Store concantenated prefix
      memcpy(child->prefix,node->prefix,min(l1,maxPrefixLength));
      child->prefixLength+=node->prefixLength+1;
    }
    *nodeRef=child;
    delete node;
  }
}

void ARTBase::eraseNode16(Node16* node,Node** nodeRef,Node** leafPlace) {
  // Delete leaf from inner node
  unsigned long pos=static_cast<unsigned long>(leafPlace-node->child);
  memmove(node->key+pos,node->key+pos+1,node->count-pos-1);
  memmove(node->child+pos,node->child+pos+1,(node->count-pos-1)*sizeof(uintptr_t));
  node->count--;

  if (node->count==3) {
    // Shrink to Node4
    Node4* newNode=new Node4();
    newNode->count=4;
    copyPrefix(node,newNode);
    for (unsigned i=0;i<4;i++)
      newNode->key[i]=flipSign(node->key[i]);
    memcpy(newNode->child,node->child,sizeof(uintptr_t)*4);
    *nodeRef=newNode;
    delete node;
  }
}

void ARTBase::eraseNode48(Node48* node,Node** nodeRef,uint8_t keyByte) {
  // Delete leaf from inner node
  node->child[node->childIndex[keyByte]]=NULL;
  node->childIndex[keyByte]=emptyMarker;
  node->count--;

  if (node->count==12) {
    // Shrink to Node16
    Node16 *newNode=new Node16();
    *nodeRef=newNode;
    copyPrefix(node,newNode);
    for (uint8_t b=0;b<=255;b++) {
      if (node->childIndex[b]!=emptyMarker) {
        newNode->key[newNode->count]=flipSign(b);
        newNode->child[newNode->count]=node->child[node->childIndex[b]];
        newNode->count++;
      }
    }
    delete node;
  }
}

void ARTBase::eraseNode256(Node256* node,Node** nodeRef,uint8_t keyByte) {
  // Delete leaf from inner node
  node->child[keyByte]=NULL;
  node->count--;

  if (node->count==37) {
    // Shrink to Node48
    Node48 *newNode=new Node48();
    *nodeRef=newNode;
    copyPrefix(node,newNode);
    for (uint8_t b=0;b<=255;b++) {
      if (node->child[b]) {
        newNode->childIndex[b]=static_cast<uint8_t>(newNode->count);
        newNode->child[newNode->count]=node->child[b];
        newNode->count++;
      }
    }
    delete node;
  }
}

ARTBase::Node** ARTBase::greaterThan(Node* n,uint8_t keyByte) const {
  // Find the next child for the keyByte
  switch (n->type) {
    case NodeType4: {
                      Node4* node=static_cast<Node4*>(n);
                      for (int i=0;i<node->count;i++) {
                        if (keyByte<node->key[i]) {
                          return &node->child[i];
                        }
                      }
                      return &nullNode;
                    }
    case NodeType16: {
                       Node16* node=static_cast<Node16*>(n);
                       for (int i=0;i<node->count;i++) {
                         if (keyByte<flipSign(node->key[i])) {
                           return &node->child[i];
                         }
                       }
                       return &nullNode;
                     }
    case NodeType48: {
                       Node48* node=static_cast<Node48*>(n);
                       for (int i=keyByte+1;i<256;i++) {
                         if (node->childIndex[i]!=emptyMarker) {
                           return &node->child[node->childIndex[i]];
                         }
                       }
                       return &nullNode;
                     }
    case NodeType256: {
                        Node256* node=static_cast<Node256*>(n);
                        for (int i=keyByte+1;i<256;i++) {
                          if (node->child[i] != NULL) {
                            return &(node->child[i]);
                          }
                        }
                        return &nullNode;
                      }
  }
  throw; // Unreachable
}

ARTBase::Node** ARTBase::lowerThan(Node* n,uint8_t keyByte) const {
  // Find the next child for the keyByte
  switch (n->type) {
    case NodeType4: {
                      Node4* node=static_cast<Node4*>(n);
                      for (int i=node->count-1;i>=0;i--) {
                        if (node->key[i]<keyByte) {
                          return &node->child[i];
                        }
                      }
                      return &nullNode;
                    }
    case NodeType16: {
                       Node16* node=static_cast<Node16*>(n);
                       for (int i=node->count-1;i>=0;i--) {
                         if (flipSign(node->key[i])<keyByte) {
                           return &node->child[i];
                         }
                       }
                       return &nullNode;
                     }
    case NodeType48: {
                       Node48* node=static_cast<Node48*>(n);
                       for (int i=keyByte-1;i>=0;i--) {
                         if (node->childIndex[i]!=emptyMarker) {
                           return &node->child[node->childIndex[i]];
                         }
                       }
                       return &nullNode;
                     }
    case NodeType256: {
                        Node256* node=static_cast<Node256*>(n);
                        for (int i=keyByte-1;i>=0;i--) {
                          if (node->child[i] != NULL) {
                            return &(node->child[i]);
                          }
                        }
                        return &nullNode;
                      }
  }
  throw; // Unreachable
}

ARTBase::Node* ARTBase::sartLookupPrefix(ARTBase::Node* node,uint8_t key[],unsigned keyLength,unsigned depth) const {
  // Find the inner node with a matching prefix

  while (node != NULL) {
    if (isLeaf(node) && keyLength >= depth) {
      return node;
    }

    unsigned matchingChars = prefixMismatch(node, key, depth, keyLength);
    if (matchingChars == keyLength) {
      // Return inner node
      return node;
    }
    else if (matchingChars != node->prefixLength) {
      return NULL;
    }
    depth += node->prefixLength;

    // find child node
    Node* child = *findChild(node, key[depth]);
    if (child) {
      node = child;
      depth++;
      continue;
    }

    // No child node found; we're at the last matching inner node
    // find the lower child instead
    child = *lowerThan(node, key[depth]);
    if (child) {
      // we need the previous page, which is either the maximum of the parent (recursive)
      // or the current minimum
      return minimum(node);
    }
    return maximum(child);
  }

  return NULL;
}

