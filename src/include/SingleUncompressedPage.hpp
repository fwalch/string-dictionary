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
    Iterator getId(uint64_t id) {
      return Iterator(this).find(id);
    }

    Iterator getString(const std::string& str) {
      return Iterator(this).find(str);
    }

    Iterator get(uint16_t delta) {
      return Iterator(this).gotoDelta(delta);
    }

    class Loader : public page::Loader<SingleUncompressedPage<TSize>> {
      public:
        void load(std::vector<std::pair<uint64_t, std::string>> values, typename page::Loader<SingleUncompressedPage<TSize>>::CallbackType const &callback) {
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

              callback(currentPage, deltaNumber++, pair.first, pair.second);
              continue;
            }

            // Write delta
            uint64_t prefixSize;
            std::string deltaValue = this->delta(*deltaRef, pair.second, prefixSize);
            this->startDelta(dataPtr);
            this->writeId(dataPtr, pair.first);
            this->writeDelta(dataPtr, deltaValue, prefixSize);

            callback(currentPage, deltaNumber++, pair.first, pair.second);
          }

          this->endPage(dataPtr);
          lastPage = currentPage;
        }
    };

    class Iterator : public page::Iterator<SingleUncompressedPage<TSize>, Iterator> {
      public:
        Iterator(SingleUncompressedPage<TSize>* pagePtr) : page::Iterator<SingleUncompressedPage<TSize>, Iterator>(pagePtr) {
        }
    };
};

#endif
