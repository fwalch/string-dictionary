#ifndef H_Page
#define H_Page

#include <cassert>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <string>
#include <vector>

namespace page {
  typedef uint8_t HeaderType;
  typedef uint64_t IdType;
  typedef uint16_t StringSizeType;
  typedef uint16_t PrefixSizeType;
  typedef std::pair<IdType, std::string> Leaf;

  enum Header : HeaderType {
    StartOfUncompressedValue = 0,
    StartOfDelta = 1,
    EndOfPage = 2
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

  template<class TPage>
    class Loader {
      public:
        typedef std::function<void(TPage*, uint16_t, IdType, std::string)> CallbackType;
        virtual ~Loader() { }
        virtual void load(std::vector<std::pair<page::IdType, std::string>> values, CallbackType callback) = 0;
      protected:
        inline std::string delta(const std::string& ref, const std::string& value, PrefixSizeType& prefixSize) {
          uint64_t pos = 0;
          while (pos < ref.size() && pos < value.size() && ref[pos] == value[pos]) {
            pos++;
          }
          assert(pos <= std::numeric_limits<PrefixSizeType>::max());
          prefixSize = static_cast<PrefixSizeType>(pos);

          auto delta = value.substr(pos, value.size()-pos);
          return delta;
        }

        // Start/End blocks
        void startPrefix(char*& dataPtr) {
          writeHeader(dataPtr, page::Header::StartOfUncompressedValue);
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
          assert(value.size() <= std::numeric_limits<StringSizeType>::max());
          page::write<StringSizeType>(dataPtr, static_cast<StringSizeType>(value.size()));
          page::writeString(dataPtr, value);
        }

        void writeDelta(char*& dataPtr, const std::string& delta, PrefixSizeType prefixSize) {
          page::write(dataPtr, prefixSize);
          writeValue(dataPtr, delta);
        }
    };

  template<class TPage>
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
          if (header == page::Header::StartOfUncompressedValue) {
            IdType id = page::read<IdType>(readPtr);
            StringSizeType size = page::read<StringSizeType>(readPtr);
            const char* value = page::readString(readPtr, size);
            this->fullString = std::string(value, value+size);

            return page::Leaf {
              id,
                this->fullString
            };
          }
          else {
            assert(header == page::Header::StartOfDelta);

            IdType id = page::read<IdType>(readPtr);
            PrefixSizeType prefixSize = page::read<PrefixSizeType>(readPtr);
            StringSizeType size = page::read<StringSizeType>(readPtr);
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
          if (header == page::Header::StartOfUncompressedValue) {
            page::advance<IdType>(this->dataPtr);
            StringSizeType size = page::read<StringSizeType>(this->dataPtr);
            page::advance(this->dataPtr, size);
          }
          else if (header == page::Header::StartOfDelta) {
            page::advance<IdType>(this->dataPtr);
            page::advance<PrefixSizeType>(this->dataPtr);
            StringSizeType size = page::read<StringSizeType>(this->dataPtr);
            page::advance(this->dataPtr, size);
          }

          char* readPtr = this->dataPtr;
          header = page::readHeader(readPtr);
          if (header == page::Header::StartOfUncompressedValue || header == page::Header::StartOfDelta) {
          }
          else if (header == page::Header::EndOfPage && this->nextPage != nullptr) {
            this->dataPtr = this->nextPage->getData();
            this->nextPage = this->nextPage->nextPage;
          }
          else {
            this->dataPtr = nullptr;
          }
          return this;
        }

        operator bool() {
          if (this->dataPtr == nullptr) {
            return false;
          }
          char* readPtr = this->dataPtr;
          page::Header header = page::readHeader(readPtr);
          return (header == page::Header::StartOfUncompressedValue || header == page::Header::StartOfDelta)
            || (header == page::Header::EndOfPage && this->nextPage != nullptr);
        }

        Iterator& find(IdType id) {
          do {
            char* readPtr = this->dataPtr;
            page::readHeader(readPtr);
            IdType foundId = page::read<IdType>(readPtr);
            if (id == foundId) {
              return *this;
            }

            page::Header header = page::readHeader(this->dataPtr);
            // Not found; skip according to header
            if (header == page::Header::StartOfUncompressedValue) {
              page::advance<IdType>(this->dataPtr);
              StringSizeType size = page::read<StringSizeType>(this->dataPtr);
              const char* value = page::readString(this->dataPtr, size);
              this->fullString = std::string(value, value+size);
            }
            else if (header == page::Header::StartOfDelta) {
              page::advance<IdType>(this->dataPtr);
              page::advance<PrefixSizeType>(this->dataPtr);
              StringSizeType size = page::read<StringSizeType>(this->dataPtr);
              page::advance(this->dataPtr, size);
            }

            readPtr = this->dataPtr;
            header = page::readHeader(readPtr);
            if (header == page::Header::StartOfUncompressedValue || header == page::Header::StartOfDelta) {
            }
            else {
              this->dataPtr = nullptr;
            }
          }
          while (this->dataPtr != nullptr);

          return *this;
        }

        Iterator& find(const std::string& searchValue) {
          uint64_t pos = 0;
          do {
            char* readPtr = this->dataPtr;
            page::Header header = page::readHeader(readPtr);
            page::read<IdType>(readPtr);

            if (header == page::Header::StartOfUncompressedValue) {
              // String is stored uncompressed
              StringSizeType size = page::read<StringSizeType>(readPtr);
              const char* value = page::readString(readPtr, size);

              assert(pos == 0);
              while(pos < size && pos < searchValue.size() && searchValue[pos] == value[pos]) {
                pos++;
              }
              if (pos == size) {
                // string found; return
                return *this;
              }
              this->fullString = std::string(value, value+size);
            }
            else if (header == page::Header::StartOfDelta) {
              PrefixSizeType prefixSize = page::read<PrefixSizeType>(readPtr);
              StringSizeType size = page::read<StringSizeType>(readPtr);
              if (prefixSize == pos && prefixSize + size == searchValue.size()) {
                // Possible match; compare characters
                const char* value = page::readString(readPtr, size);

                uint64_t searchPos = 0;
                while(searchPos+pos < searchValue.size() && searchValue[pos+searchPos] == value[searchPos]) {
                  searchPos++;
                }

                if (searchPos+pos == searchValue.size()) {
                  // string found; return
                  return *this;
                }
              }
            }

            header = page::readHeader(this->dataPtr);
            // Not found; skip according to header
            if (header == page::Header::StartOfUncompressedValue) {
              page::advance<IdType>(this->dataPtr);
              StringSizeType size = page::read<StringSizeType>(this->dataPtr);
              page::advance(this->dataPtr, size);
            }
            else if (header == page::Header::StartOfDelta) {
              page::advance<IdType>(this->dataPtr);
              page::advance<PrefixSizeType>(this->dataPtr);
              StringSizeType size = page::read<StringSizeType>(this->dataPtr);
              page::advance(this->dataPtr, size);
            }

            readPtr = this->dataPtr;
            header = page::readHeader(readPtr);
            if (header == page::Header::StartOfUncompressedValue || header == page::Header::StartOfDelta) {
            }
            else {
              this->dataPtr = nullptr;
            }
          }
          while (this->dataPtr != nullptr);

          return *this;
        }


        Iterator& gotoDelta(uint16_t delta) {
          uint16_t pos = 0;
          while (pos < delta) {
            page::Header header = page::readHeader(this->dataPtr);
            // Not found; skip according to header
            if (header == page::Header::StartOfUncompressedValue) {
              page::advance<IdType>(this->dataPtr);
              StringSizeType size = page::read<StringSizeType>(this->dataPtr);
              const char* value = page::readString(this->dataPtr, size);
              this->fullString = std::string(value, value+size);
            }
            else if (header == page::Header::StartOfDelta) {
              page::advance<IdType>(this->dataPtr);
              page::advance<PrefixSizeType>(this->dataPtr);
              StringSizeType size = page::read<StringSizeType>(this->dataPtr);
              page::advance(this->dataPtr, size);
            }

            pos++;
          }

          char* readPtr = this->dataPtr;
          page::Header header = page::readHeader(readPtr);
          if (header == page::Header::StartOfUncompressedValue || header == page::Header::StartOfDelta) {
          }
          else {
            this->dataPtr = nullptr;
          }

          return *this;
        }
    };

  template<uint64_t TSize, class TPage>
    class Page {
      public:
        const uint64_t size = TSize;
        TPage* nextPage;
        char data[TSize];

        inline char* getData() {
          return data;
        }

        Page() : nextPage(nullptr) {
        }
    };
}

template<uint64_t TSize, class TPage>
using Page = page::Page<TSize, TPage>;

template<class TPage>
using PageLoader = page::Loader<TPage>;

template<class TPage>
using PageIterator = page::Iterator<TPage>;

#endif
