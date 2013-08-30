#ifndef H_SingleUncompressedPage
#define H_SingleUncompressedPage

#include "Page.hpp"

/**
 * Fixed-size page implementation that holds a single uncompressed string per page.
 */
template<uint64_t TSize>
class SingleUncompressedPage : public Page<TSize, SingleUncompressedPage<TSize>> {
  public:
    SingleUncompressedPage() : Page<TSize, SingleUncompressedPage<TSize>>() {
    }

    SingleUncompressedPage(const SingleUncompressedPage&) = delete;
    SingleUncompressedPage& operator=(const SingleUncompressedPage&) = delete;
    PageIterator<SingleUncompressedPage<TSize>> getId(page::IdType id) {
      return Iterator(this).find(id);
    }

    PageIterator<SingleUncompressedPage<TSize>> getString(const std::string& str) {
      return Iterator(this).find(str);
    }

    PageIterator<SingleUncompressedPage<TSize>> get(uint16_t delta) {
      return Iterator(this).gotoDelta(delta);
    }

  private:
    class Iterator : public page::Iterator<SingleUncompressedPage<TSize>> {
      public:
        Iterator(SingleUncompressedPage<TSize>* pagePtr) : page::Iterator<SingleUncompressedPage<TSize>>(pagePtr) {
        }
    };

    class Loader : public page::Loader<SingleUncompressedPage<TSize>> {
      public:
        void load(std::vector<std::pair<page::IdType, std::string>> values, typename page::Loader<SingleUncompressedPage<TSize>>::CallbackType callback) {
          SingleUncompressedPage<TSize>* currentPage = nullptr;
          SingleUncompressedPage<TSize>* lastPage = nullptr;
          const char* endOfPage = nullptr;
          char* dataPtr = nullptr;
          uint16_t deltaNumber = 0;
          const uint64_t prefixHeaderSize = sizeof(page::HeaderType) + sizeof(page::IdType) + sizeof(page::StringSizeType);
          const uint64_t deltaHeaderSize = sizeof(page::HeaderType) + sizeof(page::IdType) + sizeof(page::StringSizeType) + sizeof(page::PrefixSizeType);

          const std::string* deltaRef = nullptr;
          for (const auto& pair : values) {
            if (deltaRef != nullptr) {
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
              currentPage = new SingleUncompressedPage<TSize>();
              if (lastPage != nullptr) {
                lastPage->nextPage = currentPage;
              }
              dataPtr = currentPage->data;
              endOfPage = currentPage->data + currentPage->size - sizeof(uint8_t);
              deltaNumber = 0;

              deltaRef = &pair.second;

              if (dataPtr + prefixHeaderSize + pair.second.size() > endOfPage) {
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
            page::PrefixSizeType prefixSize;
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

  public:
    static inline void load(std::vector<std::pair<page::IdType, std::string>> values, typename PageLoader<SingleUncompressedPage<TSize>>::CallbackType callback) {
      Loader().load(values, callback);
    }

    static std::string description() {
      return "a single uncompressed string per page (size " + std::to_string(TSize) + ")";
    }
};

#endif
