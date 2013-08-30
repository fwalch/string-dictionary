#ifndef H_DynamicSlottedPage
#define H_DynamicSlottedPage

#include "Page.hpp"

template<uint32_t TPrefixSize = 1>
class DynamicSlottedPage {
  private:
    class Loader;

  public:
    static uint64_t counter;
    DynamicSlottedPage<TPrefixSize>* nextPage;

    DynamicSlottedPage() = default;

    DynamicSlottedPage(const DynamicSlottedPage&) = delete;
    DynamicSlottedPage& operator=(const DynamicSlottedPage&) = delete;

    char* getData() {
      return &(reinterpret_cast<char*>(this)[sizeof(DynamicSlottedPage<TPrefixSize>)]);
    }

    PageIterator<DynamicSlottedPage<TPrefixSize>> getIndexEntry(page::IndexEntriesType indexEntry) {
      return PageIterator<DynamicSlottedPage<TPrefixSize>>(this).getIndexEntry(indexEntry);
    }

  private:
    class Loader : public page::Loader<DynamicSlottedPage<TPrefixSize>> {
      private:
        inline size_t findBlock(const uint8_t searchChar, size_t prefixPos, size_t size, std::pair<page::IdType, std::string>* values, bool& endOfString) {
          size_t start = 0;
          size_t end = size-1;

          size_t biggestFound = start;

          while (start < end) {
            size_t middle = (end+start)/2;

            if (values[middle].second[prefixPos] == searchChar) {
#ifdef DEBUG
              assert(middle >= biggestFound);
#endif
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

          uint64_t pageSize = sizeof(uintptr_t); // next page pointer
          pageSize += sizeof(HeaderType) + sizeof(IdType) + sizeof(StringSizeType) + values[0].second.size(); // Uncompressed value

          // Index section
          pageSize += sizeof(HeaderType); // Index header
          pageSize += sizeof(page::IndexEntriesType); // No of entries in index
          pageSize += (size-1)*(sizeof(OffsetType)); // Index entries

          pageSize += (size-1)*(sizeof(HeaderType)+sizeof(IdType)+sizeof(PrefixSizeType)+sizeof(StringSizeType)); // Delta headers + IDs
          // Deltas
          for (uint64_t i = 1; i < size; i++) {
            page::PrefixSizeType prefixSize;
            std::string deltaValue = this->delta(values[0].second, values[i].second, prefixSize);
            pageSize += deltaValue.size();
          }

          return pageSize + sizeof(HeaderType); // End of page header
        }

        DynamicSlottedPage<TPrefixSize>* createPage(uint64_t size, std::pair<page::IdType, std::string>* values, typename PageLoader<DynamicSlottedPage<TPrefixSize>>::CallbackType callback) {
#ifdef DEBUG
          assert(size > 0);
#endif
          counter++;

          uint64_t pageSize = getPageSize(size, values);

          char* dataPtr = new char[pageSize];
          char* originalPointer = dataPtr;

          // Write header
          page::write<uintptr_t>(dataPtr, 0L); // "next page" pointer placeholder
          // Write index
          this->startIndex(dataPtr);
          assert(size-1 <= std::numeric_limits<page::IndexEntriesType>::max());
          page::write<page::IndexEntriesType>(dataPtr, static_cast<page::IndexEntriesType>(size-1));
          page::OffsetType* indexAddress = reinterpret_cast<page::OffsetType*>(dataPtr);
          page::advance<page::OffsetType>(dataPtr, size-1);

          // Write uncompressed section
          uintptr_t startOfUncompressedSection = this->startPrefix(dataPtr);

          callback(reinterpret_cast<DynamicSlottedPage<TPrefixSize>*>(originalPointer), 0, 0, values[0].first, values[0].second);

          // Write uncompressed string
          this->writeId(dataPtr, values[0].first);
          this->writeValue(dataPtr, values[0].second);

          // Write deltas
          for (page::IndexEntriesType i = 1; i < size; i++) {
            uintptr_t valuePtr = this->startDelta(dataPtr);
            page::PrefixSizeType prefixSize;
            std::string deltaValue = this->delta(values[0].second, values[i].second, prefixSize);
            this->writeId(dataPtr, values[i].first);
            this->writeDelta(dataPtr, deltaValue, prefixSize);

            assert((valuePtr-startOfUncompressedSection) <= std::numeric_limits<page::OffsetType>::max());

            // Write to index
            // Delta number 1 is first delta (0=uncompressed),
            // is zeroth entry in array => deltaNumber-1
            indexAddress[i-1] = static_cast<page::OffsetType>(valuePtr - startOfUncompressedSection);

            callback(reinterpret_cast<DynamicSlottedPage<TPrefixSize>*>(originalPointer), i, 0, values[i].first, values[i].second);
          }

          this->endPage(dataPtr);

          return reinterpret_cast<DynamicSlottedPage<TPrefixSize>*>(originalPointer);
        }

      public:
        void load(std::vector<std::pair<page::IdType, std::string>> values, typename PageLoader<DynamicSlottedPage<TPrefixSize>>::CallbackType callback) {
          const size_t size = values.size();

          size_t start = 0;
          size_t end;
          uint32_t searchPos;
          DynamicSlottedPage<TPrefixSize>* lastPage = nullptr;

          do {
            bool endOfString = false;
            end = size-1;
            for (searchPos = 0; searchPos < TPrefixSize && !endOfString; searchPos++) {
              const uint8_t searchChar = static_cast<uint8_t>(values[start].second[searchPos]);
              if (searchChar == '\0') {
                break;
              }
              end = start+findBlock(searchChar, searchPos, end-start+1, &values[start], endOfString);
            }

            DynamicSlottedPage<TPrefixSize>* currentPage = createPage(end-start+1, &values[start], callback);

            if (lastPage != nullptr) {
              lastPage->nextPage = currentPage;
            }
            lastPage = currentPage;

            start = (start == end) ? start+1 : end+1;
          }
          while(end < size-1);
        }
    };

  public:
    static inline void load(std::vector<std::pair<page::IdType, std::string>> values, typename PageLoader<DynamicSlottedPage<TPrefixSize>>::CallbackType callback) {
      Loader().load(values, callback);
    }

    static std::string description() {
      return std::to_string(TPrefixSize);
      //return "DynamicSlotted"+std::to_string(TPrefixSize);
      //return "dynamic pages (prefix size " + std::to_string(TPrefixSize) + ")";
    }
};

template<uint32_t TPrefixSize>
uint64_t DynamicSlottedPage<TPrefixSize>::counter = 0;

#endif
