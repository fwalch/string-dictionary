#ifndef H_MultiUncompressedPage
#define H_MultiUncompressedPage

#include "Page.hpp"
#ifdef DEBUG
#undef NDEBUG
#include <cassert>
#endif

/**
 * Fixed-size page implementation that holds multiple uncompressed strings per page.
 */
template<uint64_t TSize, uint16_t TFrequency>
class MultiUncompressedPage : public Page<TSize, MultiUncompressedPage<TSize, TFrequency>> {
  private:
    static const uint8_t BitsForDelta = 3;
    static const uint16_t MaskForDelta = (1<<BitsForDelta)-1;

    static inline uint16_t decodeDelta(uint16_t value) {
      return value & MaskForDelta;
    }

    static inline uint16_t decodeOffset(uint16_t value) {
      return value >> BitsForDelta;
    }

    static inline uint16_t encodeDeltaAndOffset(uint16_t delta, uint16_t offset) {
#ifdef DEBUG
      assert(offset < (1<<(16-BitsForDelta)));
      assert(delta <= MaskForDelta);
      uint16_t value = static_cast<uint16_t>(offset << BitsForDelta) | static_cast<uint16_t>(delta & MaskForDelta);
      assert(decodeOffset(value) == offset);
      assert(decodeDelta(value) == delta);
      return value;
#else
      return static_cast<uint16_t>(offset << BitsForDelta) | static_cast<uint16_t>(delta & MaskForDelta);
#endif
    }

  public:
    static uint64_t counter;

    MultiUncompressedPage() : Page<TSize, MultiUncompressedPage<TSize, TFrequency>>() {
      counter++;
    }

    MultiUncompressedPage(const MultiUncompressedPage&) = delete;
    MultiUncompressedPage& operator=(const MultiUncompressedPage&) = delete;

    PageIterator<MultiUncompressedPage<TSize, TFrequency>> getId(page::IdType id) {
      return PageIterator<MultiUncompressedPage<TSize, TFrequency>>(this).find(id);
    }

    PageIterator<MultiUncompressedPage<TSize, TFrequency>> getString(const std::string& str) {
      return PageIterator<MultiUncompressedPage<TSize, TFrequency>>(this).find(str);
    }

    PageIterator<MultiUncompressedPage<TSize, TFrequency>> getByDelta(uint16_t deltaValue) {
      return PageIterator<MultiUncompressedPage<TSize, TFrequency>>(this).gotoDelta(deltaValue);
    }

    PageIterator<MultiUncompressedPage<TSize, TFrequency>> getByOffset(uint16_t offsetValue) {
      uint16_t offset = decodeOffset(offsetValue);
      uint16_t delta = decodeDelta(offsetValue);
      return PageIterator<MultiUncompressedPage<TSize, TFrequency>>(this).gotoOffsetWithDelta(offset, delta);
    }

  private:
    class Loader : public page::Loader<MultiUncompressedPage<TSize, TFrequency>> {
      private:
        inline static void call(typename page::Loader<MultiUncompressedPage<TSize, TFrequency>>::CallbackType callback, MultiUncompressedPage<TSize, TFrequency>* page, uint16_t absoluteDeltaNumber, uint16_t relativeDeltaNumber, uint64_t valueAddress, page::IdType id, std::string value) {
          uint64_t pageAddress = reinterpret_cast<uint64_t>(page->getData());
#ifdef DEBUG
          assert((valueAddress - pageAddress) <= std::numeric_limits<uint16_t>::max());
          assert(relativeDeltaNumber < TFrequency);
#endif
          uint16_t offset = static_cast<uint16_t>(valueAddress - pageAddress);
          callback(page, absoluteDeltaNumber, encodeDeltaAndOffset(relativeDeltaNumber, offset), id, value);
        }
      public:
        void load(std::vector<std::pair<page::IdType, std::string>> values, typename PageLoader<MultiUncompressedPage<TSize, TFrequency>>::CallbackType callback) {
          MultiUncompressedPage<TSize, TFrequency>* currentPage = nullptr;
          MultiUncompressedPage<TSize, TFrequency>* lastPage = nullptr;
          const char* endOfPage = nullptr;
          char* dataPtr = nullptr;
          uint16_t absoluteDeltaNumber = 0;
          uint16_t relativeDeltaNumber = 0;
          const uint64_t prefixHeaderSize = sizeof(page::HeaderType) + sizeof(page::IdType) + sizeof(page::StringSizeType);
          const uint64_t deltaHeaderSize = sizeof(page::HeaderType) + sizeof(page::IdType) + sizeof(page::StringSizeType) + sizeof(page::PrefixSizeType);

          const std::string* deltaRef = nullptr;
          uintptr_t valuePtr = 0;

          for (const auto& pair : values) {
            if (absoluteDeltaNumber%TFrequency == 0) {
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
              currentPage = new MultiUncompressedPage<TSize, TFrequency>();
              if (lastPage != nullptr) {
                lastPage->nextPage = currentPage;
              }
              dataPtr = currentPage->data;
              endOfPage = currentPage->data + currentPage->size - sizeof(uint8_t);
              absoluteDeltaNumber = 0;
            }

            if (absoluteDeltaNumber%TFrequency==0) {
              valuePtr = this->startPrefix(dataPtr);
              this->writeId(dataPtr, pair.first);
              this->writeValue(dataPtr, pair.second);
              deltaRef = &pair.second;
              relativeDeltaNumber = 0;
            }
            else {
#ifdef DEBUG
              assert(deltaRef != nullptr);
#endif
              page::PrefixSizeType prefixSize;
              std::string deltaValue = this->delta(*deltaRef, pair.second, prefixSize);

              this->startDelta(dataPtr);
              this->writeId(dataPtr, pair.first);
              this->writeDelta(dataPtr, deltaValue, prefixSize);
            }

#ifdef DEBUG
            assert(valuePtr != 0);
            assert(absoluteDeltaNumber < std::numeric_limits<uint16_t>::max());
#endif

            call(callback, currentPage, absoluteDeltaNumber++, relativeDeltaNumber++, valuePtr, pair.first, pair.second);
          }
          this->endPage(dataPtr);
          lastPage = currentPage;
        }
    };

  public:
    static inline void load(std::vector<std::pair<page::IdType, std::string>> values, typename PageLoader<MultiUncompressedPage<TSize, TFrequency>>::CallbackType callback) {
      Loader().load(values, callback);
    }

    static std::string description() {
      return std::to_string(TSize);
      //return "Multi"+std::to_string(TFrequency)+"-"+std::to_string(TSize);
      //return "fixed-size pages (" + std::to_string(TSize) + ") with each " + std::to_string(TFrequency) + "th string uncompressed";
    }
};

template<uint64_t TSize, uint16_t TFrequency>
uint64_t MultiUncompressedPage<TSize, TFrequency>::counter = 0;

#include "SingleUncompressedPage.hpp"
//template<uint64_t TSize>
//using SingleUncompressedPage = MultiUncompressedPage<TSize, std::numeric_limits<uint16_t>::max()>;

#endif
