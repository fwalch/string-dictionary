#ifndef H_DynamicPage
#define H_DynamicPage

#include "Page.hpp"

class DynamicPage {
  public:
    class Iterator;

    DynamicPage* nextPage;

    char* data() {
      return &(reinterpret_cast<char*>(this)[sizeof(DynamicPage)]);
    }

    Iterator getId(uint64_t id) {
      return Iterator(this).find(id);
    }

    Iterator getString(const std::string& str) {
      return Iterator(this).find(str);
    }

    class Iterator {
      protected:
        char* dataPtr;
        DynamicPage* nextPage;
        std::string fullString;

      public:
        Iterator(DynamicPage* pagePtr) : dataPtr(pagePtr->data()), nextPage(pagePtr->nextPage) {
        }

        const page::Leaf operator*() {
          std::cout << "Dereferencing Iterator" << std::endl;
          char* readPtr = this->dataPtr;
          page::Header header = page::readHeader(readPtr);
          if (header == page::Header::StartOfPrefix) {
            std::cout << "Read full string" << std::endl;
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
            std::cout << "Read delta" << std::endl;

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
          std::cout << "Increasing Iterator" << std::endl;
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
            std::cout << "Advance inside page" << std::endl;
          }
          else if (header == page::Header::EndOfPage && this->nextPage != nullptr) {
            std::cout << "Advance to next page" << std::endl;
            this->dataPtr = this->nextPage->data();
            this->nextPage = this->nextPage->nextPage;
          }
          else {
            std::cout << "Set this->dataPtr to null" << std::endl;
            this->dataPtr = nullptr;
          }
          return *this;
        }

        operator bool() {
          std::cout << "Checking condition" << std::endl;
          if (this->dataPtr == nullptr) {
            return false;
          }
          char* readPtr = this->dataPtr;
          page::Header header = page::readHeader(readPtr);
          return (header == page::Header::StartOfPrefix || header == page::Header::StartOfDelta)
            || (header == page::Header::EndOfPage && this->nextPage != nullptr);
        }

        Iterator& find(uint64_t id) {
          do {
            char* readPtr = this->dataPtr;
            page::readHeader(readPtr);
            uint64_t foundId = page::read<uint64_t>(readPtr);
            if (id == foundId) {
              return *this;
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
              std::cout << "Advance inside page" << std::endl;
            }
            else {
              std::cout << "Set this->dataPtr to null" << std::endl;
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
            page::read<uint64_t>(readPtr);

            if (header == page::Header::StartOfPrefix) {
              // String is stored uncompressed
              uint64_t size = page::read<uint64_t>(readPtr);
              const char* value = page::readString(readPtr, size);

              assert(pos == 0);
              while(pos < size && pos < searchValue.size() && searchValue[pos] == value[pos]) {
                pos++;
              }
              std::cout << "Find String search pos: " << pos << std::endl;
              if (pos == size) {
                // string found; return
                return *this;
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
                  return *this;
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
              std::cout << "Advance inside page" << std::endl;
            }
            else {
              std::cout << "Set this->dataPtr to null" << std::endl;
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
            std::cout << "Advance inside page" << std::endl;
          }
          else {
            std::cout << "Set this->dataPtr to null" << std::endl;
            this->dataPtr = nullptr;
          }

          return *this;
        }
    };
};

class DynamicPageLoader : public page::Loader {
  private:
    const uint32_t prefixSize;

    inline size_t findBlock(const uint8_t searchChar, size_t prefixPos, size_t size, std::pair<uint64_t, std::string>* values, bool& endOfString) {
      size_t start = 0;
      size_t end = size-1;

      size_t biggestFound = start;

      while (start < end) {
        size_t middle = (end+start)/2;

        if (values[middle].second[prefixPos] == searchChar) {
          assert(middle >= biggestFound);
          biggestFound = middle;
          start = middle+1;
        }
        else {
          if (values[middle].second[prefixPos] == '\0') {
            endOfString = true;
          }
          end = middle;
        }
      }

      if (values[start].second[prefixPos] != searchChar) {
        return biggestFound;
      }

      return start;
    }

    uint64_t getPageSize(uint64_t size, std::pair<page::IdType, std::string>* values) {
      using namespace page;

      uint64_t pageSize = sizeof(uint64_t); // next page pointer
      pageSize += sizeof(HeaderType) + sizeof(IdType) + sizeof(StringSizeType) + values[0].second.size(); // Uncompressed value

      pageSize += (size-1)*(sizeof(HeaderType)+sizeof(IdType)+sizeof(PrefixSizeType)+sizeof(StringSizeType)); // Delta headers + IDs
      // Deltas
      for (uint64_t i = 1; i < size; i++) {
        uint64_t prefixSize;
        std::string deltaValue = this->delta(values[0].second, values[i].second, prefixSize);
        pageSize += deltaValue.size();
      }

      return pageSize + sizeof(HeaderType); // End of page header
    }

    DynamicPage* createPage(uint64_t size, std::pair<uint64_t, std::string>* values, std::function<void(DynamicPage*, uint64_t, std::string)> callback) {
      assert(size > 0);

      uint64_t pageSize = getPageSize(size, values);

      char* dataPtr = new char[pageSize];
      callback(reinterpret_cast<DynamicPage*>(dataPtr), values[0].first, values[0].second);
      char* originalPointer = dataPtr;

      // Write header
      page::write<uint64_t>(dataPtr, 0L); // "next page" pointer placeholder
      startPrefix(dataPtr);

      // Write uncompressed string
      this->writeId(dataPtr, values[0].first);
      this->writeValue(dataPtr, values[0].second);

      // Write deltas
      for (uint64_t i = 1; i < size; i++) {
        this->startDelta(dataPtr);
        uint64_t prefixSize;
        std::string deltaValue = this->delta(values[0].second, values[i].second, prefixSize);
        this->writeId(dataPtr, values[i].first);
        this->writeDelta(dataPtr, deltaValue, prefixSize);
        callback(reinterpret_cast<DynamicPage*>(originalPointer), values[i].first, values[i].second);
      }

      endPage(dataPtr);

      return reinterpret_cast<DynamicPage*>(originalPointer);
    }

  public:
    DynamicPageLoader(uint32_t size) : prefixSize(size) {
    }

    DynamicPageLoader() : DynamicPageLoader(1) {
    }

    void load(std::vector<std::pair<uint64_t, std::string>> values, std::function<void(DynamicPage*, uint64_t, std::string)> callback) {
      using namespace std;

      const size_t size = values.size();

      size_t start = 0;
      size_t end;
      uint32_t searchPos;
      DynamicPage* lastPage = nullptr;

      do {
        bool endOfString = false;
        end = size-1;
        for (searchPos = 0; searchPos < prefixSize && !endOfString; searchPos++) {
          const uint8_t searchChar = static_cast<uint8_t>(values[start].second[searchPos]);
          if (searchChar == '\0') {
            break;
          }
          end = start+findBlock(searchChar, searchPos, end-start+1, &values[start], endOfString);
        }
        cout << "Block [" << start << "," << end << "] on position " << searchPos-1 << endl;

        DynamicPage* currentPage = createPage(end-start+1, &values[start], callback);

        if (lastPage != nullptr) {
          lastPage->nextPage = currentPage;
        }
        lastPage = currentPage;

        start = (start == end) ? start+1 : end+1;
      }
      while(end < size-1);
    }
};

#endif
