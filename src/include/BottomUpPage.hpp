#ifndef H_BottomUpPage
#define H_BottomUpPage

#include "Page.hpp"

/**
 * Fixed-size page implementation that holds a single uncompressed string per page.
 */
template<uint64_t TSize>
class BottomUpPage : public Page<TSize, BottomUpPage<TSize>> {
  public:
    static uint64_t counter;

    BottomUpPage() : Page<TSize, BottomUpPage<TSize>>() {
      counter++;
    }

    BottomUpPage(const BottomUpPage&) = delete;
    BottomUpPage& operator=(const BottomUpPage&) = delete;

    PageIterator<BottomUpPage<TSize>> getByOffset(uint16_t offset) {
      return PageIterator<BottomUpPage<TSize>>(this).skipIndex().gotoOffset(offset);
    }

    PageIterator<BottomUpPage<TSize>> find(const std::string& str) {
      return PageIterator<BottomUpPage<TSize>>(this).indexSearch(str);
    }

    PageIterator<BottomUpPage<TSize>> firstPrefix(const std::string& str) {
      return PageIterator<BottomUpPage<TSize>>(this).prefixStartIndexSearch(str);
    }

    PageIterator<BottomUpPage<TSize>> last() {
      return PageIterator<BottomUpPage<TSize>>(this).last();
    }

    PageIterator<BottomUpPage<TSize>> lastPrefix(const std::string& str) {
      return PageIterator<BottomUpPage<TSize>>(this).prefixEndIndexSearch(str);
    }

  private:
    class Loader : public page::Loader<BottomUpPage<TSize>> {
      public:
        void load(std::vector<std::pair<page::IdType, std::string>> values, typename page::Loader<BottomUpPage<TSize>>::CallbackType callback) {
          BottomUpPage<TSize>* currentPage = nullptr;
          BottomUpPage<TSize>* lastPage = nullptr;
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
          std::pair<page::IdType, std::string> lastPair;

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
              currentPage = new BottomUpPage<TSize>();
              if (lastPage != nullptr) {
                lastPage->nextPage = currentPage;
              }
              dataPtr = currentPage->data;
              endOfPage = currentPage->data + currentPage->size - sizeof(uint8_t);
              deltaNumber = 0;

              deltaRef = &pair.second;

              if (dataPtr + prefixHeaderSize+indexHeaderSize + pair.second.size() > endOfPage) {
                // We can't fit one string on this page!?
                throw;
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

              if (startOfFullString != 0 && valuePtr != 0 && lastPage != nullptr && pair.second[0] == lastPair.second[0]) {
                // "Retro-insert"
                uint64_t diff = valuePtr - startOfFullString;
#ifdef DEBUG
                assert(diff <= std::numeric_limits<uint16_t>::max());
#endif
                uint16_t offset = static_cast<uint16_t>(diff);
                //
                callback(lastPage, 0, offset, lastPair.first, lastPair.second);
              }

              startOfFullString = this->startPrefix(dataPtr);

              // Write uncompressed value
              this->writeId(dataPtr, pair.first);
              this->writeValue(dataPtr, pair.second);

              callback(currentPage, 1, /* offset*/ 0, pair.first, pair.second);
              lastPair = pair;
              deltaNumber++;
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

            uint64_t diff = valuePtr - startOfFullString;
#ifdef DEBUG
            assert(diff <= std::numeric_limits<uint16_t>::max());
#endif
            uint16_t offset = static_cast<uint16_t>(diff);
            callback(currentPage, 2, offset, pair.first, pair.second);

            deltaNumber++;
            lastPair = pair;
          }

          this->endPage(dataPtr);
          lastPage = currentPage;
        }
    };

  public:
    static inline void load(std::vector<std::pair<page::IdType, std::string>> values, typename PageLoader<BottomUpPage<TSize>>::CallbackType callback) {
      Loader().load(values, callback);
    }

    static std::string description() {
      return std::to_string(TSize);
      //return "a single uncompressed string per slotted page (size " + std::to_string(TSize) + ")";
    }
};

template<uint64_t TSize>
uint64_t BottomUpPage<TSize>::counter = 0;

#endif
