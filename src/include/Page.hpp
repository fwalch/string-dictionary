#ifndef H_Page
#define H_Page

#include <cstdint>
#include <string>
#include <cstring>
#include <cassert>
#include <iostream>

template<uint64_t TSize, uint8_t TFrequency>
class Page {
  public:
    class iterator;

    const uint64_t size = TSize;
    Page<TSize, TFrequency>* nextPage;
    char data[TSize];

    Page() : nextPage(nullptr) {
    }

    Page(const Page&) = delete;
    Page& operator=(const Page&) = delete;

    enum HeaderTypes : uint8_t {
      Prefix = 'x',
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

    iterator getString(uint32_t offset, uint64_t id) {
      return iterator(this);
    }

    iterator getId(uint32_t offset, const std::string& str) {
      return iterator(this);
    }

    static inline void writeFlag(char*& dataPtr, HeaderTypes flag) {
      write<uint8_t>(dataPtr, flag);
    }

    void startPrefix(char*& dataPtr) {
      writeFlag(dataPtr, HeaderTypes::Prefix);
    }

    void writeId(char*& dataPtr, uint64_t id) {
      write<uint64_t>(dataPtr, id);
    }

    void writeValue(char*& dataPtr, const std::string& value) {
      write<uint64_t>(dataPtr, value.size());
      writeString(dataPtr, value);
    }

    void writeDelta(char*& dataPtr, const std::string& fullString, const std::string& delta) {
      write<uint64_t>(dataPtr, fullString.size()-delta.size());
      writeValue(dataPtr, delta);
    }

    void startDelta(char*& dataPtr) {
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
        Page<TSize, TFrequency>* nextPage;
        std::string fullString;
        //TODO: string size uint64_t??

      public:
        iterator(Page<TSize, TFrequency>* pagePtr) : dataPtr(pagePtr->data), nextPage(pagePtr->nextPage) {
        }

        const LeafValue operator*() {
          std::cout << "Dereferencing iterator" << std::endl;
          char* readPtr = dataPtr;
          HeaderTypes flag = readFlag(readPtr);
          if (flag == HeaderTypes::Prefix) {
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
          if (flag == HeaderTypes::Prefix) {
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
          if (flag == HeaderTypes::Prefix || flag == HeaderTypes::Delta) {
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
          return (flag == HeaderTypes::Prefix || flag == HeaderTypes::Delta)
            || (flag == HeaderTypes::EndOfPage && nextPage != nullptr);
        }

        iterator& find(uint64_t id) {
          return *this;
        }

        iterator& find(const std::string& value) {
          return *this;
        }
    };
};

#endif
