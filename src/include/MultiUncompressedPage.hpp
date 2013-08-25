#ifndef H_MultiUncompressedPage
#define H_MultiUncompressedPage

#include "Page.hpp"

/**
 * Fixed-size page implementation that holds multiple uncompressed strings per page.
 */
template<uint64_t TSize, uint8_t TFrequency>
class MultiUncompressedPage : public Page<TSize, MultiUncompressedPage<TSize, TFrequency>> {
  public:
    class Iterator;

    MultiUncompressedPage() : Page<TSize, MultiUncompressedPage<TSize, TFrequency>>() {
    }

    MultiUncompressedPage(const MultiUncompressedPage&) = delete;
    MultiUncompressedPage& operator=(const MultiUncompressedPage&) = delete;

    Iterator getId(uint64_t id) {
      return Iterator(this).find(id);
    }

    Iterator getString(const std::string& str) {
      return Iterator(this).find(str);
    }

    class Iterator : public Page<TSize, MultiUncompressedPage<TSize, TFrequency>>::Iterator {
      public:
        Iterator(MultiUncompressedPage<TSize, TFrequency>* pagePtr) : Page<TSize, MultiUncompressedPage<TSize, TFrequency>>::Iterator(pagePtr) {
        }

        const page::Leaf operator*() {
          std::cout << "Dereferencing iterator" << std::endl;
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
          std::cout << "Increasing iterator" << std::endl;
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
            std::cout << "Set dataPtr to null" << std::endl;
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
          //TODO
          return *this;
        }

        Iterator& find(const std::string& value) {
          //TODO
          return *this;
        }

        Iterator& gotoDelta(uint16_t delta) {
          //TODO
          return *this;
        }
    };

    class Loader : public page::Loader {
      public:
        typedef std::function<void(MultiUncompressedPage<TSize, TFrequency>*, std::string, uint64_t)> CallbackType;
        void load(std::vector<std::pair<uint64_t, std::string>> values, CallbackType const &callback) {
          MultiUncompressedPage<TSize, TFrequency>* currentPage = nullptr;
          MultiUncompressedPage<TSize, TFrequency>* lastPage = nullptr;
          const char* endOfPage = nullptr;
          char* dataPtr = nullptr;
          const uint64_t prefixHeaderSize = sizeof(uint8_t) + 2*sizeof(uint64_t);
          const uint64_t deltaHeaderSize = sizeof(uint8_t) + 3*sizeof(uint64_t);

          uint64_t i = 0;
          const std::string* deltaRef = nullptr;
          for (const auto& pair : values) {
            if (i%TFrequency == 0) {
              // Will insert full string
              if (dataPtr != nullptr && dataPtr + prefixHeaderSize + pair.second.size() > endOfPage) {
                // "Finish" page
                this->endPage(dataPtr);
                lastPage = currentPage;
                currentPage = nullptr;
              }
            }
            else {
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
              currentPage = new MultiUncompressedPage<TSize, TFrequency>();
              if (lastPage != nullptr) {
                lastPage->nextPage = currentPage;
              }
              dataPtr = currentPage->data;
              endOfPage = currentPage->data + currentPage->size - sizeof(uint8_t);

              callback(currentPage, pair.second, pair.first);

              i = 0;
            }

            if (i%TFrequency==0) {
              this->startPrefix(dataPtr);
              this->writeId(dataPtr, pair.first);
              this->writeValue(dataPtr, pair.second);
              deltaRef = &pair.second;
            }
            else {
              assert(deltaRef != nullptr);
              uint64_t prefixSize;
              std::string deltaValue = this->delta(*deltaRef, pair.second, prefixSize);

              this->startDelta(dataPtr);
              this->writeId(dataPtr, pair.first);
              this->writeDelta(dataPtr, deltaValue, prefixSize);

              callback(currentPage, pair.second, pair.first);
            }

            i++;
          }
          this->endPage(dataPtr);
          lastPage = currentPage;
        }
    };
};

#endif
