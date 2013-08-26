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

    Iterator getId(page::IdType id) {
      return Iterator(this).find(id);
    }

    Iterator getString(const std::string& str) {
      return Iterator(this).find(str);
    }

    class Iterator : public page::Iterator<MultiUncompressedPage<TSize, TFrequency>, Iterator> {
      public:
        Iterator(MultiUncompressedPage<TSize, TFrequency>* pagePtr) : page::Iterator<MultiUncompressedPage<TSize, TFrequency>, Iterator>(pagePtr) {
        }
    };

    class Loader : public page::Loader<MultiUncompressedPage<TSize, TFrequency>> {
      public:
        void load(std::vector<std::pair<page::IdType, std::string>> values, typename page::Loader<MultiUncompressedPage<TSize, TFrequency>>::CallbackType const &callback) {
          MultiUncompressedPage<TSize, TFrequency>* currentPage = nullptr;
          MultiUncompressedPage<TSize, TFrequency>* lastPage = nullptr;
          const char* endOfPage = nullptr;
          char* dataPtr = nullptr;
          uint16_t deltaNumber = 0;
          const uint64_t prefixHeaderSize = sizeof(page::HeaderType) + sizeof(page::IdType) + sizeof(page::StringSizeType);
          const uint64_t deltaHeaderSize = sizeof(page::HeaderType) + sizeof(page::IdType) + sizeof(page::StringSizeType) + sizeof(page::PrefixSizeType);

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
              page::PrefixSizeType prefixSize;
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
              currentPage = new MultiUncompressedPage<TSize, TFrequency>();
              if (lastPage != nullptr) {
                lastPage->nextPage = currentPage;
              }
              dataPtr = currentPage->data;
              endOfPage = currentPage->data + currentPage->size - sizeof(uint8_t);
              deltaNumber = 0;

              callback(currentPage, deltaNumber++, pair.first, pair.second);

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
              page::PrefixSizeType prefixSize;
              std::string deltaValue = this->delta(*deltaRef, pair.second, prefixSize);

              this->startDelta(dataPtr);
              this->writeId(dataPtr, pair.first);
              this->writeDelta(dataPtr, deltaValue, prefixSize);

              callback(currentPage, deltaNumber++, pair.first, pair.second);
            }

            i++;
          }
          this->endPage(dataPtr);
          lastPage = currentPage;
        }
    };
};

#endif
