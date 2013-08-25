#ifndef H_SingleUncompressedPage
#define H_SingleUncompressedPage

#include "Page.hpp"

/**
 * Fixed-size page implementation that holds a single uncompressed string per page.
 */
template<uint64_t TSize>
class SingleUncompressedPage : public Page<TSize, SingleUncompressedPage<TSize>> {
  public:
    class Iterator;

    SingleUncompressedPage() : Page<TSize, SingleUncompressedPage<TSize>>() {
    }

    SingleUncompressedPage(const SingleUncompressedPage&) = delete;
    SingleUncompressedPage& operator=(const SingleUncompressedPage&) = delete;
    Iterator getString(uint64_t id) {
      return Iterator(this).find(id);
    }

    Iterator getId(const std::string& str) {
      return Iterator(this).find(str);
    }

    Iterator get(uint16_t delta) {
      return Iterator(this).gotoDelta(delta);
    }

    class Loader : public Page<TSize, SingleUncompressedPage<TSize>>::Loader {
      public:
        typedef std::function<void(SingleUncompressedPage<TSize>*, uint16_t, std::string, uint64_t)> CallbackType;
        void load(std::vector<std::pair<uint64_t, std::string>> values, CallbackType const &callback) {
          SingleUncompressedPage<TSize>* currentPage = nullptr;
          SingleUncompressedPage<TSize>* lastPage = nullptr;
          const char* endOfPage = nullptr;
          char* dataPtr = nullptr;
          uint16_t deltaNumber = 0;
          const uint64_t pageHeaderSize = sizeof(uint8_t) + 2*sizeof(uint64_t);
          const uint64_t deltaHeaderSize = sizeof(uint8_t) + 3*sizeof(uint64_t);

          const std::string* deltaRef = nullptr;
          for (const auto& pair : values) {
            if (deltaRef != nullptr) {
              // Will insert delta
              uint64_t prefixSize;
              std::string deltaValue = this->delta(*deltaRef, pair.second, prefixSize);
              if (dataPtr != nullptr && dataPtr + deltaHeaderSize + deltaValue.size() > endOfPage) {
                // "Finish" page
                this->endPage(dataPtr);
                lastPage = currentPage;
                currentPage = nullptr;
              }
            }

            if (currentPage == nullptr) {
              // Create new page
              std::cout << "Create page" << std::endl;
              currentPage = new SingleUncompressedPage<TSize>();
              if (lastPage != nullptr) {
                lastPage->nextPage = currentPage;
              }
              dataPtr = currentPage->data;
              endOfPage = currentPage->data + currentPage->size - sizeof(uint8_t);
              deltaNumber = 0;

              deltaRef = &pair.second;

              if (dataPtr + pageHeaderSize + pair.second.size() > endOfPage) {
                std::cout << "Cannot write one uncompressed string to the page" << std::endl;
                assert(false);
              }

              this->startPrefix(dataPtr);

              // Write uncompressed value
              this->writeId(dataPtr, pair.first);
              this->writeValue(dataPtr, pair.second);

              callback(currentPage, deltaNumber++, pair.second, pair.first);
              continue;
            }

            // Write delta
            uint64_t prefixSize;
            std::string deltaValue = this->delta(*deltaRef, pair.second, prefixSize);
            this->startDelta(dataPtr);
            this->writeId(dataPtr, pair.first);
            this->writeDelta(dataPtr, deltaValue, prefixSize);

            callback(currentPage, deltaNumber++, pair.second, pair.first);
          }

          this->endPage(dataPtr);
          lastPage = currentPage;
        }
    };

    class Iterator : public Page<TSize, SingleUncompressedPage<TSize>>::Iterator {
      public:
        Iterator(SingleUncompressedPage<TSize>* pagePtr) : Page<TSize, SingleUncompressedPage<TSize>>::Iterator(pagePtr) {
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
            this->dataPtr = this->nextPage->data;
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

#endif
