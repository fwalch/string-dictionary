#ifndef H_SlottedPage
#define H_SlottedPage

#include "Page.hpp"

/**
 * Fixed-size page implementation that holds a single uncompressed string per page.
 */
template<uint64_t TSize>
class SlottedPage : public Page<TSize, SlottedPage<TSize>> {
  public:
    static uint64_t counter;

    SlottedPage() : Page<TSize, SlottedPage<TSize>>() {
      counter++;
    }

    SlottedPage(const SlottedPage&) = delete;
    SlottedPage& operator=(const SlottedPage&) = delete;

    PageIterator<SlottedPage<TSize>> getIndexEntry(page::IndexEntriesType indexEntry) {
      return PageIterator<SlottedPage<TSize>>(this).getIndexEntry(indexEntry);
    }

  private:
    class Loader : public page::Loader<SlottedPage<TSize>> {
      public:
        void load(std::vector<std::pair<page::IdType, std::string>> values, typename page::Loader<SlottedPage<TSize>>::CallbackType callback) {
          SlottedPage<TSize>* currentPage = nullptr;
          SlottedPage<TSize>* lastPage = nullptr;
          const char* endOfPage = nullptr;
          char* dataPtr = nullptr;
          page::IndexEntriesType deltaNumber = 0;
          const uint64_t prefixHeaderSize = sizeof(page::HeaderType) + sizeof(page::IdType) + sizeof(page::StringSizeType);
          const uint64_t deltaHeaderSize = sizeof(page::HeaderType) + sizeof(page::IdType) + sizeof(page::StringSizeType) + sizeof(page::PrefixSizeType);
          const uint64_t indexHeaderSize = sizeof(page::HeaderType) + sizeof(page::IndexEntriesType);

          const std::string* deltaRef = nullptr;
          uintptr_t startOfFullString = 0;
          uintptr_t valuePtr = 0;
          page::IndexEntriesType numberOfDeltas = 0;

          for (auto pairIt = values.cbegin(); pairIt != values.cend(); ++pairIt) {
            const auto& pair = *pairIt;
            if (deltaRef != nullptr) {
              // Would like to insert delta
              if (deltaNumber == numberOfDeltas+1) {
                // Can't fit delta; "finish" page
                this->endPage(dataPtr);
                lastPage = currentPage;
                currentPage = nullptr;
              }
            }

            if (currentPage == nullptr) {
              // Create new page
              currentPage = new SlottedPage<TSize>();
              if (lastPage != nullptr) {
                lastPage->nextPage = currentPage;
              }
              dataPtr = currentPage->data;
              endOfPage = currentPage->data + currentPage->size - sizeof(uint8_t);
              deltaNumber = 0;

              deltaRef = &pair.second;

              if (dataPtr + prefixHeaderSize+indexHeaderSize + pair.second.size() > endOfPage) {
                // We can't fit one string on this page!?
                throw Exception("Can't fit on page: " + pair.second);
              }

              // Count how many deltas will fit on this page
              auto pageSize = dataPtr+prefixHeaderSize+pair.second.size()+indexHeaderSize;
              auto deltaIt = pairIt;
              ++deltaIt;
              numberOfDeltas = 0;
              if (deltaIt != values.cend()) {
                auto deltaSize = this->deltaLength(*deltaRef, deltaIt->second);
                pageSize += sizeof(page::OffsetType)+deltaHeaderSize+deltaSize;
                while (pageSize <= endOfPage && ++deltaIt != values.cend()) {
                  numberOfDeltas++;

                  deltaSize = this->deltaLength(*deltaRef, deltaIt->second);
                  pageSize += sizeof(page::OffsetType)+deltaHeaderSize+deltaSize;
                }
              }

              this->startIndex(dataPtr);

              page::write<page::IndexEntriesType>(dataPtr, numberOfDeltas);

              // Reserve space for the index
              page::advance<page::OffsetType>(dataPtr, numberOfDeltas);

              startOfFullString = this->startPrefix(dataPtr);

              // Write uncompressed value
              this->writeId(dataPtr, pair.first);
              this->writeValue(dataPtr, pair.second);

              callback(currentPage, deltaNumber++, 0, pair.first, pair.second);
              continue;
            }


            // Write delta
            page::PrefixSizeType prefixSize;
            std::string deltaValue = this->delta(*deltaRef, pair.second, prefixSize);
            valuePtr = this->startDelta(dataPtr);
            this->writeId(dataPtr, pair.first);
            this->writeDelta(dataPtr, deltaValue, prefixSize);

#ifdef DEBUG
            assert(startOfFullString != 0);
            assert((valuePtr-startOfFullString) <= std::numeric_limits<page::OffsetType>::max());
#endif
            // Write offset relative to start of full values to index
            page::OffsetType* indexAddress = reinterpret_cast<page::OffsetType*>(currentPage->data + sizeof(page::HeaderType) + sizeof(page::IndexEntriesType));
            // Delta number 1 is first delta (0=uncompressed),
            // is zeroth entry in array => deltaNumber-1
            indexAddress[deltaNumber-1] = static_cast<page::OffsetType>(valuePtr - startOfFullString);

            callback(currentPage, deltaNumber, 0, pair.first, pair.second);

            deltaNumber++;
          }

          this->endPage(dataPtr);
          lastPage = currentPage;
        }
    };

  public:
    static inline void load(std::vector<std::pair<page::IdType, std::string>> values, typename PageLoader<SlottedPage<TSize>>::CallbackType callback) {
      Loader().load(values, callback);
    }

    static std::string description() {
      return std::to_string(TSize);
      //return "a single uncompressed string per slotted page (size " + std::to_string(TSize) + ")";
    }
};

template<uint64_t TSize>
uint64_t SlottedPage<TSize>::counter = 0;

#endif
