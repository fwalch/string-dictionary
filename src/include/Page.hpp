#ifndef H_Page
#define H_Page

#include <cstdint>
#include <string>
#include <cstring>
#include <cassert>
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

  template<class TPage>
  class Loader {
    public:
      typedef std::function<void(TPage*, uint16_t, IdType, std::string)> CallbackType;
    protected:
      inline std::string delta(const std::string& ref, const std::string& value, uint64_t& pos) {
        pos = 0;
        while (pos < ref.size() && pos < value.size() && ref[pos] == value[pos]) {
          pos++;
        }

        auto delta = value.substr(pos, value.size()-pos);
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

  template<class TPage, class TIterator>
  class Iterator {
    protected:
      char* dataPtr;
      TPage* nextPage;
      std::string fullString;

      Iterator(char* data, TPage* next) : dataPtr(data), nextPage(next) {
      }

      Iterator(TPage* pagePtr) : Iterator(pagePtr->data, pagePtr->nextPage) {
      }

    public:
      const page::Leaf operator*() {
        char* readPtr = this->dataPtr;
        page::Header header = page::readHeader(readPtr);
        if (header == page::Header::StartOfPrefix) {
          uint64_t id = page::read<uint64_t>(readPtr);
          uint64_t size = page::read<uint64_t>(readPtr);
          const char* value = page::readString(readPtr, size);
          this->fullString = std::string(value, value+size);

          return page::Leaf {
            id,
              this->fullString
          };
        }
        else {
          assert(header == page::Header::StartOfDelta);

          uint64_t id = page::read<uint64_t>(readPtr);
          uint64_t prefixSize = page::read<uint64_t>(readPtr);
          uint64_t size = page::read<uint64_t>(readPtr);
          const char* value = page::readString(readPtr, size);

          std::string output;
          output.reserve(prefixSize + size +1);
          //TODO: not efficient
          output.insert(0, this->fullString.substr(0, prefixSize));
          output.insert(prefixSize, value, size);
          output[prefixSize+size] = '\0';

          return page::Leaf {
            id,
              output
          };
        }
      }

      Iterator& operator++() {
        page::Header header = page::readHeader(this->dataPtr);
        if (header == page::Header::StartOfPrefix) {
          page::advance<uint64_t>(this->dataPtr);
          uint64_t size = page::read<uint64_t>(this->dataPtr);
          page::advance(this->dataPtr, size);
        }
        else if (header == page::Header::StartOfDelta) {
          page::advance<uint64_t>(this->dataPtr);
          page::advance<uint64_t>(this->dataPtr);
          uint64_t size = page::read<uint64_t>(this->dataPtr);
          page::advance(this->dataPtr, size);
        }

        char* readPtr = this->dataPtr;
        header = page::readHeader(readPtr);
        if (header == page::Header::StartOfPrefix || header == page::Header::StartOfDelta) {
        }
        else if (header == page::Header::EndOfPage && this->nextPage != nullptr) {
          this->dataPtr = this->nextPage->getData();
          this->nextPage = this->nextPage->nextPage;
        }
        else {
          this->dataPtr = nullptr;
        }
        return *(reinterpret_cast<TIterator*>(this));
      }

      operator bool() {
        if (this->dataPtr == nullptr) {
          return false;
        }
        char* readPtr = this->dataPtr;
        page::Header header = page::readHeader(readPtr);
        return (header == page::Header::StartOfPrefix || header == page::Header::StartOfDelta)
          || (header == page::Header::EndOfPage && this->nextPage != nullptr);
      }

      TIterator& find(uint64_t id) {
        do {
          char* readPtr = this->dataPtr;
          page::readHeader(readPtr);
          uint64_t foundId = page::read<uint64_t>(readPtr);
          if (id == foundId) {
            return *(reinterpret_cast<TIterator*>(this));
          }

          page::Header header = page::readHeader(this->dataPtr);
          // Not found; skip according to header
          if (header == page::Header::StartOfPrefix) {
            page::advance<uint64_t>(this->dataPtr);
            uint64_t size = page::read<uint64_t>(this->dataPtr);
            const char* value = page::readString(this->dataPtr, size);
            this->fullString = std::string(value, value+size);
          }
          else if (header == page::Header::StartOfDelta) {
            page::advance<uint64_t>(this->dataPtr);
            page::advance<uint64_t>(this->dataPtr);
            uint64_t size = page::read<uint64_t>(this->dataPtr);
            page::advance(this->dataPtr, size);
          }

          readPtr = this->dataPtr;
          header = page::readHeader(readPtr);
          if (header == page::Header::StartOfPrefix || header == page::Header::StartOfDelta) {
          }
          else {
            this->dataPtr = nullptr;
          }
        }
        while (this->dataPtr != nullptr);

        return *(reinterpret_cast<TIterator*>(this));
      }

      TIterator& find(const std::string& searchValue) {
        uint64_t pos = 0;
        do {
          char* readPtr = this->dataPtr;
          page::Header header = page::readHeader(readPtr);
          page::read<uint64_t>(readPtr);

          if (header == page::Header::StartOfPrefix) {
            // String is stored uncompressed
            uint64_t size = page::read<uint64_t>(readPtr);
            const char* value = page::readString(readPtr, size);

            assert(pos == 0);
            while(pos < size && pos < searchValue.size() && searchValue[pos] == value[pos]) {
              pos++;
            }
            if (pos == size) {
              // string found; return
              return *(reinterpret_cast<TIterator*>(this));
            }
            this->fullString = std::string(value, value+size);
          }
          else if (header == page::Header::StartOfDelta) {
            uint64_t prefixSize = page::read<uint64_t>(readPtr);
            uint64_t size = page::read<uint64_t>(readPtr);
            if (prefixSize == pos && prefixSize + size == searchValue.size()) {
              // Possible match; compare characters
              const char* value = page::readString(readPtr, size);

              uint64_t searchPos = 0;
              while(searchPos+pos < searchValue.size() && searchValue[pos+searchPos] == value[searchPos]) {
                searchPos++;
              }

              if (searchPos+pos == searchValue.size()) {
                // string found; return
                return *(reinterpret_cast<TIterator*>(this));
              }
            }
          }

          header = page::readHeader(this->dataPtr);
          // Not found; skip according to header
          if (header == page::Header::StartOfPrefix) {
            page::advance<uint64_t>(this->dataPtr);
            uint64_t size = page::read<uint64_t>(this->dataPtr);
            page::advance(this->dataPtr, size);
          }
          else if (header == page::Header::StartOfDelta) {
            page::advance<uint64_t>(this->dataPtr);
            page::advance<uint64_t>(this->dataPtr);
            uint64_t size = page::read<uint64_t>(this->dataPtr);
            page::advance(this->dataPtr, size);
          }

          readPtr = this->dataPtr;
          header = page::readHeader(readPtr);
          if (header == page::Header::StartOfPrefix || header == page::Header::StartOfDelta) {
          }
          else {
            this->dataPtr = nullptr;
          }
        }
        while (this->dataPtr != nullptr);

        return *(reinterpret_cast<TIterator*>(this));
      }


      TIterator& gotoDelta(uint16_t delta) {
        uint16_t pos = 0;
        while (pos < delta) {
          page::Header header = page::readHeader(this->dataPtr);
          // Not found; skip according to header
          if (header == page::Header::StartOfPrefix) {
            page::advance<uint64_t>(this->dataPtr);
            uint64_t size = page::read<uint64_t>(this->dataPtr);
            const char* value = page::readString(this->dataPtr, size);
            this->fullString = std::string(value, value+size);
          }
          else if (header == page::Header::StartOfDelta) {
            page::advance<uint64_t>(this->dataPtr);
            page::advance<uint64_t>(this->dataPtr);
            uint64_t size = page::read<uint64_t>(this->dataPtr);
            page::advance(this->dataPtr, size);
          }

          pos++;
        }

        char* readPtr = this->dataPtr;
        page::Header header = page::readHeader(readPtr);
        if (header == page::Header::StartOfPrefix || header == page::Header::StartOfDelta) {
        }
        else {
          this->dataPtr = nullptr;
        }

        return *(reinterpret_cast<TIterator*>(this));
      }
  };

}

template<uint64_t TSize, class TPage>
class Page {
  public:
    TPage* nextPage;
    char data[TSize];

    inline char* getData() {
      return data;
    }

    Page() : nextPage(nullptr) {
    }

    const uint64_t size = TSize;
};

#endif
