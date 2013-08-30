#ifndef H_SingleUncompressedPage
#define H_SingleUncompressedPage

#include "Page.hpp"

/**
 * Fixed-size page implementation that holds a single uncompressed string per page.
 */
template<uint64_t TSize>
class SingleUncompressedPage : public Page<TSize, SingleUncompressedPage<TSize>> {
  public:
    static uint64_t counter;

    SingleUncompressedPage() : Page<TSize, SingleUncompressedPage<TSize>>() {
      counter++;
    }

    SingleUncompressedPage(const SingleUncompressedPage&) = delete;
    SingleUncompressedPage& operator=(const SingleUncompressedPage&) = delete;
    PageIterator<SingleUncompressedPage<TSize>> getId(page::IdType id) {
      return PageIterator<SingleUncompressedPage<TSize>>(this).find(id);
    }

    PageIterator<SingleUncompressedPage<TSize>> getString(const std::string& str) {
      return PageIterator<SingleUncompressedPage<TSize>>(this).find(str);
    }

    PageIterator<SingleUncompressedPage<TSize>> getByDelta(uint16_t delta) {
      return PageIterator<SingleUncompressedPage<TSize>>(this).gotoDelta(delta);
    }

    PageIterator<SingleUncompressedPage<TSize>> getByOffset(uint16_t offset) {
      return PageIterator<SingleUncompressedPage<TSize>>(this).gotoOffset(offset);
    }

  private:
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
          uintptr_t valuePtr;

          for (const auto& pair : values) {
            if (deltaRef != nullptr) {
              // Will insert delta
              auto deltaSize = this->deltaLength(*deltaRef, pair.second);
              if (dataPtr != nullptr && dataPtr + deltaHeaderSize + deltaSize > endOfPage) {
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
                // We can't fit one string on this page!?
                throw Exception("Can't fit on page: " + pair.second);
              }

              valuePtr = this->startPrefix(dataPtr);

              // Write uncompressed value
              this->writeId(dataPtr, pair.first);
              this->writeValue(dataPtr, pair.second);

              page::Loader<SingleUncompressedPage<TSize>>::call(callback, currentPage, deltaNumber++, valuePtr, pair.first, pair.second);
              continue;
            }

            // Write delta
            page::PrefixSizeType prefixSize;
            std::string deltaValue = this->delta(*deltaRef, pair.second, prefixSize);
            valuePtr = this->startDelta(dataPtr);
            this->writeId(dataPtr, pair.first);
            this->writeDelta(dataPtr, deltaValue, prefixSize);

            page::Loader<SingleUncompressedPage<TSize>>::call(callback, currentPage, deltaNumber++, valuePtr, pair.first, pair.second);
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
      return std::to_string(TSize);
      //return "a single uncompressed string per page (size " + std::to_string(TSize) + ")";
    }
};

template<uint64_t TSize>
uint64_t SingleUncompressedPage<TSize>::counter = 0;

#endif
