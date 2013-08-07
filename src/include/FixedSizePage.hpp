#ifndef H_FixedSizePage
#define H_FixedSizePage

#include <cstdint>
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>

template<uint64_t TSize>
class FixedSizePage {
  public:
    class iterator;

    const uint64_t size = TSize;
    FixedSizePage<TSize>* nextPage;
    uint64_t pageId;
    char data[TSize];

    FixedSizePage(uint64_t id) : nextPage(nullptr), pageId(id) {
    }

    FixedSizePage(const FixedSizePage&) = delete;
    FixedSizePage& operator=(const FixedSizePage&) = delete;

    enum HeaderTypes : uint8_t {
      StartOfPage = 'x',
      Delta = 'y',
      EndOfPage = 'z'
    };

    static inline const char* readString(char*& dataPtr, uint64_t size) {
      const char* value = const_cast<const char*>(dataPtr);
      dataPtr += size;
      std::cout << "Reading string " << std::string(value, value+size) << std::endl;
      return value;
    }

    template<class T> static inline void write(char*& dataPtr, T value) {
      *reinterpret_cast<T*>(dataPtr) = value;
      dataPtr += sizeof(T);
      std::cout << "Writing " << value << std::endl;
    }

    template<class T> static inline T read(char*& dataPtr) {
      T value = *reinterpret_cast<T*>(dataPtr);
      dataPtr += sizeof(T);
      std::cout << "Reading " << value << std::endl;
      return value;
    }

    static inline void writeString(char*& dataPtr, const std::string& value) {
      memcpy(dataPtr, value.c_str(), value.size());
      std::cout << "Writing string " << value << std::endl;
      dataPtr += value.size();
    }

    iterator getString(uint64_t id) {
      return iterator(this).find(id);
    }

    iterator get(uint16_t delta) {
      return iterator(this).gotoDelta(delta);
    }

    iterator getId(const std::string& str) {
      return iterator(this).find(str);
    }

    static inline void writeFlag(char*& dataPtr, HeaderTypes flag) {
      write<uint8_t>(dataPtr, flag);
    }

    void beginPage(char*& dataPtr) {
      writeFlag(dataPtr, HeaderTypes::StartOfPage);
    }

    void writeId(char*& dataPtr, uint64_t id) {
      write<uint64_t>(dataPtr, id);
    }

    void writeValue(char*& dataPtr, const std::string& value) {
      write<uint64_t>(dataPtr, value.size());
      writeString(dataPtr, value);
    }

    void writeDelta(char*& dataPtr, const std::string& fullString, const std::string& delta, uint64_t prefixSize) {
      write<uint64_t>(dataPtr, prefixSize);
      writeValue(dataPtr, delta);
    }

    void beginDelta(char*& dataPtr) {
      writeFlag(dataPtr, HeaderTypes::Delta);
    }

    static inline HeaderTypes readFlag(char*& dataPtr) {
      return static_cast<HeaderTypes>(read<uint8_t>(dataPtr));
    }

    void endPage(char*& dataPtr) {
      writeFlag(dataPtr, HeaderTypes::EndOfPage);
    }

    struct LeafValue {
      uint64_t id;
      std::string value;
    };

    class iterator {
      private:
        char* dataPtr;
        FixedSizePage<TSize>* nextPage;
        std::string fullString;

      public:
        iterator(FixedSizePage<TSize>* pagePtr) : dataPtr(pagePtr->data), nextPage(pagePtr->nextPage) {
        }

        const LeafValue operator*() {
          std::cout << "Dereferencing iterator" << std::endl;
          char* readPtr = dataPtr;
          HeaderTypes flag = readFlag(readPtr);
          if (flag == HeaderTypes::StartOfPage) {
            std::cout << "Read full string" << std::endl;
            uint64_t id = read<uint64_t>(readPtr);
            uint64_t size = read<uint64_t>(readPtr);
            const char* value = readString(readPtr, size);
            fullString = std::string(value, value+size);

            return LeafValue {
              id,
              fullString
            };
          }
          else {
            assert(flag == HeaderTypes::Delta);
            std::cout << "Read delta" << std::endl;

            uint64_t id = read<uint64_t>(readPtr);
            uint64_t prefixSize = read<uint64_t>(readPtr);
            uint64_t size = read<uint64_t>(readPtr);
            const char* value = readString(readPtr, size);

            std::string output;
            output.reserve(prefixSize + size +1);
            //TODO: not efficient
            output.insert(0, fullString.substr(0, prefixSize));
            output.insert(prefixSize, value, size);
            output[prefixSize+size] = '\0';

            return LeafValue {
              id,
              output
            };
          }
        }

        template<class T> void advance() {
          dataPtr += sizeof(T);
        }

        void advance(uint64_t size) {
          dataPtr += size;
        }

        iterator& operator++() {
          std::cout << "Increasing iterator" << std::endl;
          HeaderTypes flag = readFlag(dataPtr);
          if (flag == HeaderTypes::StartOfPage) {
            advance<uint64_t>();
            uint64_t size = read<uint64_t>(dataPtr);
            advance(size);
          }
          else if (flag == HeaderTypes::Delta) {
            advance<uint64_t>();
            advance<uint64_t>();
            uint64_t size = read<uint64_t>(dataPtr);
            advance(size);
          }

          char* readPtr = dataPtr;
          flag = readFlag(readPtr);
          if (flag == HeaderTypes::StartOfPage || flag == HeaderTypes::Delta) {
            std::cout << "Advance inside page" << std::endl;
          }
          else if (flag == HeaderTypes::EndOfPage && nextPage != nullptr) {
            std::cout << "Advance to next page" << std::endl;
            dataPtr = nextPage->data;
            nextPage = nextPage->nextPage;
          }
          else {
            std::cout << "Set dataPtr to null" << std::endl;
            dataPtr = nullptr;
          }
          return *this;
        }

        operator bool() {
          std::cout << "Checking condition" << std::endl;
          if (dataPtr == nullptr) {
            return false;
          }
          char* readPtr = dataPtr;
          HeaderTypes flag = readFlag(readPtr);
          return (flag == HeaderTypes::StartOfPage || flag == HeaderTypes::Delta)
            || (flag == HeaderTypes::EndOfPage && nextPage != nullptr);
        }

        iterator& find(uint64_t id) {
          do {
            char* readPtr = dataPtr;
            readFlag(readPtr);
            uint64_t foundId = read<uint64_t>(readPtr);
            if (id == foundId) {
              return *this;
            }

            HeaderTypes flag = readFlag(dataPtr);
            // Not found; skip according to flag
            if (flag == HeaderTypes::StartOfPage) {
              advance<uint64_t>();
              uint64_t size = read<uint64_t>(dataPtr);
              const char* value = readString(dataPtr, size);
              fullString = std::string(value, value+size);
            }
            else if (flag == HeaderTypes::Delta) {
              advance<uint64_t>();
              advance<uint64_t>();
              uint64_t size = read<uint64_t>(dataPtr);
              advance(size);
            }

            readPtr = dataPtr;
            flag = readFlag(readPtr);
            if (flag == HeaderTypes::StartOfPage || flag == HeaderTypes::Delta) {
              std::cout << "Advance inside page" << std::endl;
            }
            else {
              std::cout << "Set dataPtr to null" << std::endl;
              dataPtr = nullptr;
            }
          }
          while (dataPtr != nullptr);

          return *this;
        }

        iterator& find(const std::string& searchValue) {
          uint64_t pos = 0;
          do {
            char* readPtr = dataPtr;
            HeaderTypes flag = readFlag(readPtr);
            read<uint64_t>(readPtr);

            if (flag == HeaderTypes::StartOfPage) {
              // String is stored uncompressed
              uint64_t size = read<uint64_t>(readPtr);
              const char* value = readString(readPtr, size);

              assert(pos == 0);
              while(pos < size && pos < searchValue.size() && searchValue[pos] == value[pos]) {
                pos++;
              }
              std::cout << "Find String search pos: " << pos << std::endl;
              if (pos == size) {
                // string found; return
                return *this;
              }
              fullString = std::string(value, value+size);
            }
            else if (flag == HeaderTypes::Delta) {
              uint64_t prefixSize = read<uint64_t>(readPtr);
              uint64_t size = read<uint64_t>(readPtr);
              if (prefixSize == pos && prefixSize + size == searchValue.size()) {
                // Possible match; compare characters
                const char* value = readString(readPtr, size);

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

            flag = readFlag(dataPtr);
            // Not found; skip according to flag
            if (flag == HeaderTypes::StartOfPage) {
              advance<uint64_t>();
              uint64_t size = read<uint64_t>(dataPtr);
              advance(size);
            }
            else if (flag == HeaderTypes::Delta) {
              advance<uint64_t>();
              advance<uint64_t>();
              uint64_t size = read<uint64_t>(dataPtr);
              advance(size);
            }

            readPtr = dataPtr;
            flag = readFlag(readPtr);
            if (flag == HeaderTypes::StartOfPage || flag == HeaderTypes::Delta) {
              std::cout << "Advance inside page" << std::endl;
            }
            else {
              std::cout << "Set dataPtr to null" << std::endl;
              dataPtr = nullptr;
            }
          }
          while (dataPtr != nullptr);

          return *this;
        }

        iterator& gotoDelta(uint16_t delta) {
          uint16_t pos = 0;
          while (pos < delta) {
            HeaderTypes flag = readFlag(dataPtr);
            // Not found; skip according to flag
            if (flag == HeaderTypes::StartOfPage) {
              advance<uint64_t>();
              uint64_t size = read<uint64_t>(dataPtr);
              const char* value = readString(dataPtr, size);
              fullString = std::string(value, value+size);
            }
            else if (flag == HeaderTypes::Delta) {
              advance<uint64_t>();
              advance<uint64_t>();
              uint64_t size = read<uint64_t>(dataPtr);
              advance(size);
            }

            pos++;
          }

          char* readPtr = dataPtr;
          HeaderTypes flag = readFlag(readPtr);
          if (flag == HeaderTypes::StartOfPage || flag == HeaderTypes::Delta) {
            std::cout << "Advance inside page" << std::endl;
          }
          else {
            std::cout << "Set dataPtr to null" << std::endl;
            dataPtr = nullptr;
          }

          return *this;
        }
    };
};

#endif
