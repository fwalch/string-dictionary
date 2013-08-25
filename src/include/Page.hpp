#ifndef H_Page
#define H_Page

#include <cstdint>
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>
#include <vector>
#include <functional>

namespace page {
  typedef uint8_t HeaderType;
  typedef uint64_t IdType;
  typedef uint64_t StringSizeType;
  typedef uint64_t PrefixSizeType;

  enum Header : HeaderType {
    StartOfPrefix = 'x',
    StartOfDelta = 'y',
    EndOfPage = 'z'
  };

  // Advance

  template<class T> void advance(char*& dataPtr) {
    dataPtr += sizeof(T);
  }

  static inline void advance(char*& dataPtr, uint64_t size) {
    dataPtr += size;
  }

  // Read

  template<class T> static inline T read(char*& dataPtr) {
    T value = *reinterpret_cast<T*>(dataPtr);
    dataPtr += sizeof(T);
    return value;
  }

  static inline const char* readString(char*& dataPtr, StringSizeType size) {
    const char* value = const_cast<const char*>(dataPtr);
    dataPtr += size;
    return value;
  }

  static inline Header readHeader(char*& dataPtr) {
    return static_cast<Header>(read<HeaderType>(dataPtr));
  }

  // Write

  template<class T> static inline void write(char*& dataPtr, T value) {
    *reinterpret_cast<T*>(dataPtr) = value;
    dataPtr += sizeof(T);
  }

  static inline void writeString(char*& dataPtr, const std::string& value) {
    memcpy(dataPtr, value.c_str(), value.size());
    dataPtr += value.size();
  }

  static inline void writeHeader(char*& dataPtr, Header flag) {
    write<HeaderType>(dataPtr, flag);
  }

  struct Leaf {
    uint64_t id;
    std::string value;
  };

  class Loader {
    protected:
      inline std::string delta(const std::string& ref, const std::string& value, uint64_t& pos) {
        pos = 0;
        while (pos < ref.size() && pos < value.size() && ref[pos] == value[pos]) {
          pos++;
        }

        auto delta = value.substr(pos, value.size()-pos);
        std::cout << "Delta between " << ref << " & " << value << ":" << delta << std::endl;
        return delta;
      }

      // Start/End blocks
      void startPrefix(char*& dataPtr) {
        writeHeader(dataPtr, page::Header::StartOfPrefix);
      }

      void startDelta(char*& dataPtr) {
        writeHeader(dataPtr, page::Header::StartOfDelta);
      }

      void endPage(char*& dataPtr) {
        writeHeader(dataPtr, page::Header::EndOfPage);
      }

      // Write values
      void writeId(char*& dataPtr, page::IdType id) {
        page::write<page::IdType>(dataPtr, id);
      }

      void writeValue(char*& dataPtr, const std::string& value) {
        page::write<uint64_t>(dataPtr, value.size());
        page::writeString(dataPtr, value);
      }

      void writeDelta(char*& dataPtr, const std::string& delta, uint64_t prefixSize) {
        page::write<uint64_t>(dataPtr, prefixSize);
        writeValue(dataPtr, delta);
      }
  };
}

template<uint64_t TSize, class TType>
class Page {
  protected:
    TType* nextPage;
    char data[TSize];

    Page() : nextPage(nullptr) {
    }

  public:
    const uint64_t size = TSize;

    class Iterator {
      protected:
        char* dataPtr;
        TType* nextPage;
        std::string fullString;

      public:
        Iterator(TType* pagePtr) : dataPtr(pagePtr->data), nextPage(pagePtr->nextPage) {
        }
    };

};

#endif
