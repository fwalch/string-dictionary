#include "ReverseIndexMART.hpp"

#include <stdlib.h>    // malloc, free
#include <string.h>    // memset, memcpy
#include <emmintrin.h> // x86 SSE intrinsics
#include <stdio.h>
#include <assert.h>
#include <sys/time.h>  // gettime
#include <algorithm>   // std::random_shuffle
#include <cassert>
#include <cstring>
#include <iostream>

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

ReverseIndexMART::ReverseIndexMART(IndexART& lookup) : nextId(0), tree(NULL), maxKeyLength(0), index(lookup) {
}

ReverseIndexMART::~ReverseIndexMART() {
}

inline uint8_t* cast(string& str) {
  return reinterpret_cast<uint8_t*>(const_cast<char*>(str.c_str()));
}

// Constants for the node types
static const int8_t NodeType4=0;
static const int8_t NodeType16=1;
static const int8_t NodeType48=2;
static const int8_t NodeType256=3;

// Shared header of all inner nodes
struct ReverseIndexMART::Node {
  // length of the compressed path (prefix)
  uint32_t prefixLength;
  // number of non-null children
  uint16_t count;
  // node type
  int8_t type;
  // compressed path (prefix)
  uint8_t* prefix;

  Node(int8_t nodeType, uint32_t prefLen) : prefixLength(prefLen),count(0),type(nodeType) {
    prefix = new uint8_t[prefixLength];
    memset(prefix,0,prefixLength);
  }

  ~Node() {
    delete[] prefix;
  }
};

// Node with up to 4 children
struct ReverseIndexMART::Node4 : Node {
  uint8_t key[4];
  Node* child[4];

  Node4(uint32_t len) : Node(NodeType4, len) {
    memset(key,0,sizeof(key));
    memset(child,0,sizeof(child));
  }
};

// Node with up to 16 children
struct ReverseIndexMART::Node16 : Node {
  uint8_t key[16];
  Node* child[16];

  Node16(uint32_t len) : Node(NodeType16, len) {
    memset(key,0,sizeof(key));
    memset(child,0,sizeof(child));
  }
};

static const uint8_t emptyMarker=48;

// Node with up to 48 children
struct ReverseIndexMART::Node48 : Node {
  uint8_t childIndex[256];
  Node* child[48];

  Node48(uint32_t len) : Node(NodeType48, len) {
    memset(childIndex,emptyMarker,sizeof(childIndex));
    memset(child,0,sizeof(child));
  }
};

// Node with up to 256 children
struct ReverseIndexMART::Node256 : Node {
  Node* child[256];

  Node256(uint32_t len) : Node(NodeType256, len) {
    memset(child,0,sizeof(child));
  }
};

inline ReverseIndexMART::Node* ReverseIndexMART::makeLeaf(uintptr_t tid) {
  // Create a pseudo-leaf
  return reinterpret_cast<Node*>((tid<<1)|1);
}

uintptr_t ReverseIndexMART::getLeafValue(Node* node) {
  // The the value stored in the pseudo-leaf
  return reinterpret_cast<uintptr_t>(node)>>1;
}

bool ReverseIndexMART::isLeaf(Node* node) {
  // Is the node a leaf?
  return reinterpret_cast<uintptr_t>(node)&1;
}

inline uint8_t flipSign(uint8_t keyByte);
uint8_t flipSign(uint8_t keyByte) {
  // Flip the sign bit, enables signed SSE comparison of unsigned values, used by Node16
  return keyByte^128;
}

// This address is used to communicate that search failed
ReverseIndexMART::Node* ReverseIndexMART::nullNode=NULL;

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

ReverseIndexMART::Node** ReverseIndexMART::findChild(Node* n,uint8_t keyByte) {
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

ReverseIndexMART::Node* ReverseIndexMART::minimum(ReverseIndexMART::Node* node) {
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

ReverseIndexMART::Node* ReverseIndexMART::maximum(ReverseIndexMART::Node* node) {
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

bool ReverseIndexMART::leafMatches(Node* leaf,uint8_t key[],unsigned keyLength,unsigned depth) {
  // Check if the key of the leaf is equal to the searched key
  if (depth!=keyLength) {
    uint8_t* leafKey = loadKey(getLeafValue(leaf));
    for (unsigned i=depth;i<keyLength;i++)
      if (leafKey[i]!=key[i])
        return false;
  }
  return true;
}

unsigned ReverseIndexMART::prefixMismatch(Node* node,uint8_t key[],unsigned depth) {
  // Compare the key with the prefix of the node, return the number matching bytes
  unsigned pos;
    for (pos=0;pos<node->prefixLength;pos++)
      if (key[depth+pos]!=node->prefix[pos])
        return pos;
  return pos;
}

ReverseIndexMART::Node* ReverseIndexMART::lookupValue(ReverseIndexMART::Node* node,uint8_t key[],unsigned keyLength,unsigned depth) {
  // Find the node with a matching key, alternative pessimistic version

  while (node!=NULL) {
    if (isLeaf(node)) {
      if (leafMatches(node,key,keyLength,depth)) {
        return node;
      }
      return NULL;
    }

    if (prefixMismatch(node,key,depth)!=node->prefixLength) {
      return NULL;
    } else {
      depth+=node->prefixLength;
    }

    node=*findChild(node,key[depth]);
    depth++;
  }

  return NULL;
}

inline unsigned min(unsigned a,unsigned b);
unsigned min(unsigned a,unsigned b) {
  // Helper function
  return (a<b)?a:b;
}

void ReverseIndexMART::copyPrefix(ReverseIndexMART::Node* src,ReverseIndexMART::Node* dst) {
  // Helper function that copies the prefix from the source to the destination node
  dst->prefixLength=src->prefixLength;
  memcpy(dst->prefix,src->prefix,src->prefixLength);
}

void ReverseIndexMART::insertValue(Node* node,Node** nodeRef,uint8_t key[],unsigned depth,uintptr_t value) {
  // Insert the leaf value into the tree

  if (node==NULL) {
    *nodeRef=makeLeaf(value);
    return;
  }

  if (isLeaf(node)) {
    // Replace leaf with Node4 and store both leaves in it
    uint8_t* existingKey = loadKey(getLeafValue(node));
    unsigned newPrefixLength=0;
    while (existingKey[depth+newPrefixLength]==key[depth+newPrefixLength])
      newPrefixLength++;

    Node4* newNode=new Node4(newPrefixLength);
    memcpy(newNode->prefix,key+depth,newPrefixLength);
    *nodeRef=newNode;

    insertNode4(newNode,nodeRef,existingKey[depth+newPrefixLength],node);
    insertNode4(newNode,nodeRef,key[depth+newPrefixLength],makeLeaf(value));
    return;
  }

  // Handle prefix of inner node
  if (node->prefixLength) {
    unsigned mismatchPos=prefixMismatch(node,key,depth);
    if (mismatchPos!=node->prefixLength) {
      // Prefix differs, create new node
      Node4* newNode=new Node4(mismatchPos);
      *nodeRef=newNode;
      memcpy(newNode->prefix,node->prefix,mismatchPos);
      // Break up prefix
      insertNode4(newNode,nodeRef,node->prefix[mismatchPos],node);
      node->prefixLength-=(mismatchPos+1);
      memmove(node->prefix,node->prefix+mismatchPos+1,node->prefixLength);
      insertNode4(newNode,nodeRef,key[depth+mismatchPos],makeLeaf(value));
      return;
    }
    depth+=node->prefixLength;
  }

  // Recurse
  Node** child=findChild(node,key[depth]);
  if (*child) {
    insertValue(*child,child,key,depth+1,value);
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

void ReverseIndexMART::insertNode4(Node4* node,Node** nodeRef,uint8_t keyByte,Node* child) {
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
    Node16* newNode=new Node16(node->prefixLength);
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

void ReverseIndexMART::insertNode16(Node16* node,Node** nodeRef,uint8_t keyByte,Node* child) {
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
    Node48* newNode=new Node48(node->prefixLength);
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

void ReverseIndexMART::insertNode48(Node48* node,Node** nodeRef,uint8_t keyByte,Node* child) {
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
    Node256* newNode=new Node256(node->prefixLength);
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

void ReverseIndexMART::insertNode256(Node256* node,uint8_t keyByte,Node* child) {
  // Insert leaf into inner node
  node->count++;
  node->child[keyByte]=child;
}

// Forward references

void ReverseIndexMART::erase(Node* node,Node** nodeRef,uint8_t key[],unsigned keyLength,unsigned depth) {
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
    if (prefixMismatch(node,key,depth)!=node->prefixLength)
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
    erase(*child,child,key,keyLength,depth+1);
  }
}

void ReverseIndexMART::eraseNode4(Node4* node,Node** nodeRef,Node** leafPlace) {
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
      node->prefix[l1]=node->key[0];
      l1++;

      unsigned l2=child->prefixLength;
      memcpy(node->prefix+l1,child->prefix,l2);
      l1+=l2;
      // Store concantenated prefix
      memcpy(child->prefix,node->prefix,l1);
      child->prefixLength+=node->prefixLength+1;
    }
    *nodeRef=child;
    delete node;
  }
}

void ReverseIndexMART::eraseNode16(Node16* node,Node** nodeRef,Node** leafPlace) {
  // Delete leaf from inner node
  unsigned long pos=static_cast<unsigned long>(leafPlace-node->child);
  memmove(node->key+pos,node->key+pos+1,node->count-pos-1);
  memmove(node->child+pos,node->child+pos+1,(node->count-pos-1)*sizeof(uintptr_t));
  node->count--;

  if (node->count==3) {
    // Shrink to Node4
    Node4* newNode=new Node4(node->prefixLength);
    newNode->count=4;
    copyPrefix(node,newNode);
    for (unsigned i=0;i<4;i++)
      newNode->key[i]=flipSign(node->key[i]);
    memcpy(newNode->child,node->child,sizeof(uintptr_t)*4);
    *nodeRef=newNode;
    delete node;
  }
}

void ReverseIndexMART::eraseNode48(Node48* node,Node** nodeRef,uint8_t keyByte) {
  // Delete leaf from inner node
  node->child[node->childIndex[keyByte]]=NULL;
  node->childIndex[keyByte]=emptyMarker;
  node->count--;

  if (node->count==12) {
    // Shrink to Node16
    Node16 *newNode=new Node16(node->prefixLength);
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

void ReverseIndexMART::eraseNode256(Node256* node,Node** nodeRef,uint8_t keyByte) {
  // Delete leaf from inner node
  node->child[keyByte]=NULL;
  node->count--;

  if (node->count==37) {
    // Shrink to Node48
    Node48 *newNode=new Node48(node->prefixLength);
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

void ReverseIndexMART::insert(std::string key, uint64_t value) {
  unsigned strlen = static_cast<unsigned>(key.size() + 1);
  if (strlen > maxKeyLength) {
    maxKeyLength = strlen;
  }

  insertValue(tree, &tree, cast(key), 0, value);
}

inline size_t findBlock(const char searchChar, size_t prefixPos, size_t size, string* values) {
  size_t start = 0;
  size_t end = size-1;

  size_t biggestFound = start;

  while (start < end) {
    size_t middle = (end+start)/2;

    if (values[middle][prefixPos] == searchChar) {
      assert(middle >= biggestFound);
      biggestFound = middle;
      start = middle+1;
    }
    else {
      assert(values[middle][prefixPos] > searchChar);
      end = middle;
    }
  }

  if (values[start][prefixPos] != searchChar) {
    return biggestFound;
  }

  return start;
}

ReverseIndexMART::Node* ReverseIndexMART::createRootNode(size_t size, string* values, size_t& searchPos) {
  const size_t start = 0;
  const size_t end = size-1;
  searchPos = 0;

  size_t max = values[start].size()+1;

  // Advance prefix as long as characters match
  while (searchPos < max && values[start][searchPos] == values[end][searchPos]) {
    searchPos++;
  }

  uint32_t globalPrefixLength = static_cast<uint32_t>(searchPos);
  if (globalPrefixLength > 0) {
    cout << "Global prefix: " << string(&values[start][0], globalPrefixLength) << endl;
  }
  else {
    cout << "No global prefix" << endl;
  }

  Node4* root = new Node4(globalPrefixLength);
  memcpy(root->prefix, &values[start][0], globalPrefixLength);

  return root;
}

void ReverseIndexMART::bulkInsertRec(size_t size, string* values, size_t searchPos, Node* node, Node** nodeRef) {
  size_t start = 0;
  size_t end = 0;
  size_t keyBytePos;

  do {
    const char searchChar = values[start][searchPos];
    cout << "Searching for " << searchChar << " at " << searchPos << " (" << start << "-" << size << ")" << endl;
    end = start+findBlock(searchChar, searchPos, size-start, &values[start]);
    cout << "Found " << end-start+1 << " strings of " << size << endl;

    keyBytePos = searchPos++;

    size_t max = values[start].size()+1;

    // Advance prefix as long as characters match
    while (searchPos < max && values[start][searchPos] == values[end][searchPos]) {
      searchPos++;
    }

    uint32_t prefixLength = static_cast<uint32_t>(searchPos-keyBytePos)-1;
    const char* prefix = prefixLength > 0
      ? values[start].substr(keyBytePos+1, prefixLength).c_str()
      : nullptr;
    uint8_t keyByte = static_cast<uint8_t>(values[start][keyBytePos]);

    if (start != end) {
      // Create inner node for common prefix of this chunk of strings
      cout << ":: Prefix node" << endl;
      cout << "Key from parent: " << keyByte << endl;
      if (prefixLength > 0) {
        cout << "Prefix: " << prefixLength << "-" << string(prefix, prefixLength) << endl;
      }
      else {
        cout << "No prefix" << endl;
      }

      Node4* newNode = new Node4(prefixLength);
      memcpy(newNode->prefix, prefix, prefixLength);

      switch (node->type) {
        case NodeType4: insertNode4(static_cast<Node4*>(node),nodeRef,keyByte,newNode); break;
        case NodeType16: insertNode16(static_cast<Node16*>(node),nodeRef,keyByte,newNode); break;
        case NodeType48: insertNode48(static_cast<Node48*>(node),nodeRef,keyByte,newNode); break;
        case NodeType256: insertNode256(static_cast<Node256*>(node),keyByte,newNode); break;
      }

      bulkInsertRec(end-start+1, &values[start], keyBytePos+1, newNode, reinterpret_cast<Node**>(&newNode));

      // Advance to next chunks of strings
      start = end+1;
    }
    else {
      // Only one string left; create suffix node and leaf
      cout << ":: Suffix node + Leaf" << endl;
      Node* newNode = makeLeaf(nextId++);

      if (prefixLength > 0) {
        // Create suffix node
        Node4* intermediate = new Node4(prefixLength-1);
        memcpy(intermediate->prefix, prefix, prefixLength-1);
        char keyToChild = values[start][keyBytePos+prefixLength];
        insertNode4(intermediate, nodeRef, static_cast<uint8_t>(keyToChild), newNode);
        newNode = intermediate;

        cout << "Intermediate node" << endl;
        cout << "Key from parent: " << keyByte << endl;
        if (prefixLength > 1) {
          cout << "Prefix: " << prefixLength-1 << "-" << string(prefix, prefixLength-1) << endl;
        }
        else {
          cout << "No prefix" << endl;
        }
        cout << "Key to leaf: " << keyToChild << endl;
      }
      else {
        cout << "Direct to leaf" << endl;
        cout << "Key to leaf: " << keyByte << endl;
      }

      switch (node->type) {
        case NodeType4: insertNode4(static_cast<Node4*>(node),nodeRef,keyByte,newNode); break;
        case NodeType16: insertNode16(static_cast<Node16*>(node),nodeRef,keyByte,newNode); break;
        case NodeType48: insertNode48(static_cast<Node48*>(node),nodeRef,keyByte,newNode); break;
        case NodeType256: insertNode256(static_cast<Node256*>(node),keyByte,newNode); break;
      }

      // Go to next string in prefix (parent) chunk or to new chunk
      start++;
    }

    // Revert prefix search position for next string/chunk
    searchPos = keyBytePos;
  }
  while(end < size-1);
}

void ReverseIndexMART::bulkInsert(size_t size, string* values) {
  size_t searchPos;
  tree = createRootNode(size, values, searchPos);
  bulkInsertRec(size, values, searchPos, tree, &tree);
}

bool ReverseIndexMART::lookup(std::string key, uint64_t& value) {
  unsigned strlen = static_cast<unsigned>(key.size() + 1);
  Node* leaf = lookupValue(tree, cast(key), strlen, 0);

  if (leaf == NULL) {
    return false;
  }

  assert(isLeaf(leaf));
  value = static_cast<uint64_t>(getLeafValue(leaf));

  return true;
}

uint8_t* ReverseIndexMART::loadKey(uintptr_t tid) {
  string str = index.values[tid].second;
  return cast(str);
}
