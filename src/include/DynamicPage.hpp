#ifndef H_DynamicPage
#define H_DynamicPage

#include "Page.hpp"

template<uint32_t TPrefixSize = 1>
class DynamicPage {
  private:
    class Loader;

  public:
    DynamicPage<TPrefixSize>* nextPage;

    DynamicPage() = default;

    DynamicPage(const DynamicPage&) = delete;
    DynamicPage& operator=(const DynamicPage&) = delete;

    char* getData() {
      return &(reinterpret_cast<char*>(this)[sizeof(DynamicPage<TPrefixSize>)]);
    }

    PageIterator<DynamicPage<TPrefixSize>> getId(page::IdType id) {
      return Iterator(this).find(id);
    }

    PageIterator<DynamicPage<TPrefixSize>> getString(const std::string& str) {
      return Iterator(this).find(str);
    }

    PageIterator<DynamicPage<TPrefixSize>> get(uint16_t delta) {
      return Iterator(this).gotoDelta(delta);
    }

  private:
    class Iterator : public page::Iterator<DynamicPage<TPrefixSize>> {
      public:
        Iterator(DynamicPage<TPrefixSize>* pagePtr) : page::Iterator<DynamicPage<TPrefixSize>>(pagePtr->getData(), pagePtr->nextPage) {
        }
    };

    class Loader : public page::Loader<DynamicPage<TPrefixSize>> {
      private:
        inline size_t findBlock(const uint8_t searchChar, size_t prefixPos, size_t size, std::pair<page::IdType, std::string>* values, bool& endOfString) {
          size_t start = 0;
          size_t end = size-1;

          size_t biggestFound = start;

          while (start < end) {
            size_t middle = (end+start)/2;

            if (values[middle].second[prefixPos] == searchChar) {
              assert(middle >= biggestFound);
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

          pageSize += (size-1)*(sizeof(HeaderType)+sizeof(IdType)+sizeof(PrefixSizeType)+sizeof(StringSizeType)); // Delta headers + IDs
          // Deltas
          for (uint64_t i = 1; i < size; i++) {
            page::PrefixSizeType prefixSize;
            std::string deltaValue = this->delta(values[0].second, values[i].second, prefixSize);
            pageSize += deltaValue.size();
          }

          return pageSize + sizeof(HeaderType); // End of page header
        }

        DynamicPage<TPrefixSize>* createPage(uint64_t size, std::pair<page::IdType, std::string>* values, typename PageLoader<DynamicPage<TPrefixSize>>::CallbackType callback) {
          assert(size > 0);

          uint64_t pageSize = getPageSize(size, values);
          uint16_t deltaNumber = 0;

          char* dataPtr = new char[pageSize];
          callback(reinterpret_cast<DynamicPage<TPrefixSize>*>(dataPtr), deltaNumber++, values[0].first, values[0].second);
          char* originalPointer = dataPtr;

          // Write header
          page::write<uintptr_t>(dataPtr, 0L); // "next page" pointer placeholder
          this->startPrefix(dataPtr);

          // Write uncompressed string
          this->writeId(dataPtr, values[0].first);
          this->writeValue(dataPtr, values[0].second);

          // Write deltas
          for (uint64_t i = 1; i < size; i++) {
            this->startDelta(dataPtr);
            page::PrefixSizeType prefixSize;
            std::string deltaValue = this->delta(values[0].second, values[i].second, prefixSize);
            this->writeId(dataPtr, values[i].first);
            this->writeDelta(dataPtr, deltaValue, prefixSize);
            callback(reinterpret_cast<DynamicPage<TPrefixSize>*>(originalPointer), deltaNumber++, values[i].first, values[i].second);
          }

          this->endPage(dataPtr);

          return reinterpret_cast<DynamicPage<TPrefixSize>*>(originalPointer);
        }

      public:
        void load(std::vector<std::pair<page::IdType, std::string>> values, typename PageLoader<DynamicPage<TPrefixSize>>::CallbackType callback) {
          const size_t size = values.size();

          size_t start = 0;
          size_t end;
          uint32_t searchPos;
          DynamicPage<TPrefixSize>* lastPage = nullptr;

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

            DynamicPage<TPrefixSize>* currentPage = createPage(end-start+1, &values[start], callback);

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
    static inline void load(std::vector<std::pair<page::IdType, std::string>> values, typename PageLoader<DynamicPage<TPrefixSize>>::CallbackType callback) {
      Loader().load(values, callback);
    }

    static std::string description() {
      return "dynamic pages (prefix size " + std::to_string(TPrefixSize) + ")";
    }
};

#endif
