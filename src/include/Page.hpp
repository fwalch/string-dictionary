#ifndef H_Page
#define H_Page

#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <string>
#include <vector>
#undef NDEBUG
#include <cassert>
#include "Exception.hpp"
#include <iostream>

namespace page {
  typedef uint8_t HeaderType;
  typedef uint64_t IdType;
  typedef uint16_t StringSizeType;
  typedef uint16_t OffsetType; // For bottom-up/SART
  //typedef uint32_t OffsetType; // For indirect indexing
  typedef uint16_t IndexEntriesType;
  typedef uint16_t PrefixSizeType;
  typedef std::pair<IdType, std::string> Leaf;

  enum Header : HeaderType {
    StartOfUncompressedValue = 0,
    StartOfDelta = 1,
    StartOfIndex = 2,
    EndOfPage = 3,
  };

  // Advance

  template<class T> void advance(char*& dataPtr) {
    dataPtr += sizeof(T);
  }

  template<class T> void advance(char*& dataPtr, uint64_t count) {
    dataPtr += (sizeof(T) * count);
  }

  static inline void advance(char*& dataPtr, uint64_t size) {
    dataPtr += size;
  }

  // Read

  template<class T> static inline T read(char*& dataPtr) {
    T value = *reinterpret_cast<T*>(dataPtr);
    dataPtr += sizeof(T);
    return value;
  }

  static inline const char* readString(char*& dataPtr, StringSizeType size) {
    const char* value = const_cast<const char*>(dataPtr);
    dataPtr += size;
    return value;
  }

  static inline Header readHeader(char*& dataPtr) {
    return static_cast<Header>(read<HeaderType>(dataPtr));
  }

  static inline void skipIndex(char*& dataPtr) {
    assert(page::readHeader(dataPtr) == page::Header::StartOfIndex);
    page::IndexEntriesType indexEntries = page::read<page::IndexEntriesType>(dataPtr);
    page::advance<OffsetType>(dataPtr, indexEntries);
#ifdef DEBUG
    char* readPtr = dataPtr;
    assert(page::readHeader(readPtr) == page::Header::StartOfUncompressedValue);
#endif
  }

  // Write

  template<class T> static inline void write(char*& dataPtr, T value) {
    *reinterpret_cast<T*>(dataPtr) = value;
    dataPtr += sizeof(T);
  }

  static inline void writeString(char*& dataPtr, const std::string& value) {
    memcpy(dataPtr, value.c_str(), value.size());
    dataPtr += value.size();
  }

  static inline void writeHeader(char*& dataPtr, Header flag) {
    write<HeaderType>(dataPtr, flag);
  }

  inline PrefixSizeType prefixLength(const char* ref, size_t refSize, const std::string& value) {
    uint64_t pos = 0;
    while (pos < refSize && pos < value.size() && ref[pos] == value[pos]) {
      pos++;
    }
#ifdef DEBUG
    assert(pos <= std::numeric_limits<PrefixSizeType>::max());
    assert(value.size()-pos <= std::numeric_limits<StringSizeType>::max());
#endif
    return static_cast<PrefixSizeType>(pos);
  }

  template<typename T>
    inline T min(T v1, T v2) {
      return v1 > v2 ? v2 : v1;
    }

  static inline std::string delta(const std::string& ref, const std::string& value, PrefixSizeType& prefixSize) {
    uint64_t pos = 0;
    while (pos < ref.size() && pos < value.size() && ref[pos] == value[pos]) {
      pos++;
    }
#ifdef DEBUG
    assert(pos <= std::numeric_limits<PrefixSizeType>::max());
#endif
    prefixSize = static_cast<PrefixSizeType>(pos);

    auto delta = value.substr(pos, value.size()-pos);
    return delta;
  }

  template<class TPage>
    class Loader {
      public:
        typedef std::function<void(TPage*, page::IndexEntriesType, uint16_t, IdType, std::string)> CallbackType;
        virtual ~Loader() { }
        virtual void load(std::vector<std::pair<page::IdType, std::string>> values, CallbackType callback) = 0;
      protected:
        inline static void call(CallbackType callback, TPage* page, uint16_t deltaNumber, uint64_t valueAddress, IdType id, std::string value) {
          uint64_t pageAddress = reinterpret_cast<uint64_t>(page->getData());
#ifdef DEBUG
          assert((valueAddress - pageAddress) <= std::numeric_limits<uint16_t>::max());
#endif
          uint16_t offset = static_cast<uint16_t>(valueAddress - pageAddress);
          callback(page, deltaNumber, offset, id, value);
        }

        inline StringSizeType deltaLength(const std::string& ref, const std::string& value) {
          uint64_t pos = 0;
          while (pos < ref.size() && pos < value.size() && ref[pos] == value[pos]) {
            pos++;
          }
#ifdef DEBUG
          assert(pos <= std::numeric_limits<PrefixSizeType>::max());
          assert(value.size()-pos <= std::numeric_limits<StringSizeType>::max());
#endif
          return static_cast<StringSizeType>(value.size()-pos);
        }

        inline std::string delta(const std::string& ref, const std::string& value, PrefixSizeType& prefixSize) {
          return page::delta(ref, value, prefixSize);
        }

        // Start/End blocks
        uintptr_t startPrefix(char*& dataPtr) {
          uintptr_t ptrVal = reinterpret_cast<uintptr_t>(dataPtr);
          writeHeader(dataPtr, page::Header::StartOfUncompressedValue);
          return ptrVal;
        }

        uintptr_t startIndex(char*& dataPtr) {
          uintptr_t ptrVal = reinterpret_cast<uintptr_t>(dataPtr);
          writeHeader(dataPtr, page::Header::StartOfIndex);
          return ptrVal;
        }

        uintptr_t startDelta(char*& dataPtr) {
          uintptr_t ptrVal = reinterpret_cast<uintptr_t>(dataPtr);
          writeHeader(dataPtr, page::Header::StartOfDelta);
          return ptrVal;
        }

        void endPage(char*& dataPtr) {
          writeHeader(dataPtr, page::Header::EndOfPage);
        }

        // Write values
        void writeId(char*& dataPtr, page::IdType id) {
          page::write<page::IdType>(dataPtr, id);
        }

        void writeValue(char*& dataPtr, const std::string& value) {
#ifdef DEBUG
          assert(value.size() <= std::numeric_limits<StringSizeType>::max());
#endif
          page::write<StringSizeType>(dataPtr, static_cast<StringSizeType>(value.size()));
          page::writeString(dataPtr, value);
        }

        void writeDelta(char*& dataPtr, const std::string& delta, PrefixSizeType prefixSize) {
          page::write(dataPtr, prefixSize);
          writeValue(dataPtr, delta);
        }
    };

  template<class TPage>
    class Iterator {
      friend TPage;

      protected:
      char* dataPtr;
      TPage* currentPage;
      TPage* nextPage;
      char* startOfFullString;

      public:
      Iterator() : dataPtr(nullptr), currentPage(nullptr), nextPage(nullptr), startOfFullString(nullptr) {
      }

      Iterator(TPage* pagePtr) : dataPtr(pagePtr->getData()), currentPage(pagePtr), nextPage(pagePtr->nextPage), startOfFullString(nullptr) {
      }

      IdType getId() {
        assert(this->dataPtr != nullptr);
        char* readPtr = this->dataPtr;
        this->dataPtr = nullptr;

        page::Header header = page::readHeader(readPtr);

        if (header != page::Header::StartOfUncompressedValue) {
          assert(header == page::Header::StartOfDelta);
        }

        return page::read<IdType>(readPtr);
      }

      std::string getValue() {
        assert(this->dataPtr != nullptr);
        char* readPtr = this->dataPtr;
        this->dataPtr = nullptr;

        page::Header header = page::readHeader(readPtr);
        if (header == page::Header::StartOfUncompressedValue) {
          page::advance<IdType>(readPtr);
          StringSizeType size = page::read<StringSizeType>(readPtr);
          const char* value = page::readString(readPtr, size);
          return std::string(value, size);
        }
        else {
          assert(header == page::Header::StartOfDelta);

          page::advance<IdType>(readPtr);
          PrefixSizeType prefixSize = page::read<PrefixSizeType>(readPtr);
          StringSizeType size = page::read<StringSizeType>(readPtr);
          const char* value = page::readString(readPtr, size);

          std::string output;
          output.reserve(prefixSize + size + 1);
          assert(startOfFullString != nullptr);
          char* readPtr = startOfFullString;
          const char* fullValue = page::readString(readPtr, prefixSize);

          output.insert(0, fullValue, prefixSize);
          output.insert(prefixSize, value, size);
          output[prefixSize+size] = '\0';

          return output;
        }
      }

      const page::Leaf operator*() {
        assert(this->dataPtr != nullptr);
        char* readPtr = this->dataPtr;
        page::Header header = page::readHeader(readPtr);
        if (header == page::Header::StartOfUncompressedValue) {
          IdType id = page::read<IdType>(readPtr);
          StringSizeType size = page::read<StringSizeType>(readPtr);
          startOfFullString = readPtr;
          const char* value = page::readString(readPtr, size);

          return page::Leaf {
            id,
              std::string(value, size)
          };
        }
        else {
          assert(header == page::Header::StartOfDelta);

          IdType id = page::read<IdType>(readPtr);
          PrefixSizeType prefixSize = page::read<PrefixSizeType>(readPtr);
          StringSizeType size = page::read<StringSizeType>(readPtr);
          const char* value = page::readString(readPtr, size);

          std::string output;
          output.reserve(prefixSize + size +1);
          assert(startOfFullString != nullptr);
          char* readPtr = startOfFullString;
          const char* fullValue = page::readString(readPtr, prefixSize);

          output.insert(0, fullValue, prefixSize);
          output.insert(prefixSize, value, size);
          output[prefixSize+size] = '\0';

          return page::Leaf {
            id,
              output
          };
        }
      }

      Iterator& operator++() {
        assert(this->dataPtr != nullptr);
        page::Header header = page::readHeader(this->dataPtr);
        if (header == page::Header::StartOfUncompressedValue) {
          page::advance<IdType>(this->dataPtr);
          StringSizeType size = page::read<StringSizeType>(this->dataPtr);
          page::advance(this->dataPtr, size);
        }
        else if (header == page::Header::StartOfDelta) {
          page::advance<IdType>(this->dataPtr);
          page::advance<PrefixSizeType>(this->dataPtr);
          StringSizeType size = page::read<StringSizeType>(this->dataPtr);
          page::advance(this->dataPtr, size);
        }

        char* readPtr = this->dataPtr;
        header = page::readHeader(readPtr);
        if (header == page::Header::StartOfUncompressedValue || header == page::Header::StartOfDelta) {
        }
        else if (header == page::Header::EndOfPage && this->nextPage != nullptr) {
          this->dataPtr = this->nextPage->getData();
          this->nextPage = this->nextPage->nextPage;
          char* readPtr = this->dataPtr;
          if (page::readHeader(readPtr) == page::Header::StartOfIndex) {
            page::skipIndex(this->dataPtr);
          }
        }
        else {
          this->dataPtr = nullptr;
        }
        return *this;
      }

      operator bool() {
        if (this->dataPtr == nullptr) {
          return false;
        }

        char* readPtr = this->dataPtr;
        page::Header header = page::readHeader(readPtr);
        return (header == page::Header::StartOfUncompressedValue || header == page::Header::StartOfDelta)
          || (header == page::Header::EndOfPage && this->nextPage != nullptr);
      }

      Iterator& skipIndex() {
        assert(this->dataPtr != nullptr);

        page::skipIndex(this->dataPtr);

        return *this;
      }

      Iterator& prefixEndIndexSearch(const std::string& str) {
        assert(this->dataPtr != nullptr);

        assert(page::readHeader(this->dataPtr) == page::Header::StartOfIndex);
        page::IndexEntriesType indexEntries = page::read<page::IndexEntriesType>(this->dataPtr);

        OffsetType* indexPtr = reinterpret_cast<OffsetType*>(this->dataPtr);
        page::advance<OffsetType>(this->dataPtr, indexEntries);
        char* startOfUncompressedSection = this->dataPtr;

        assert(page::readHeader(this->dataPtr) == page::Header::StartOfUncompressedValue);
        page::advance<IdType>(this->dataPtr);
        StringSizeType fullStringSize = page::read<StringSizeType>(this->dataPtr);
        startOfFullString = this->dataPtr;
        const char* fullString = page::readString(this->dataPtr, fullStringSize);
        PrefixSizeType prefixSize = prefixLength(fullString, fullStringSize, str);

        char* uncompressedGoodPtr;
        if (prefixSize == str.size()) {
          uncompressedGoodPtr = startOfUncompressedSection;
        }
        else {
          uncompressedGoodPtr = nullptr;
        }

        char* lastGoodPtr = nullptr;
        if (indexEntries > 0) {
          // Perform binary search
          page::IndexEntriesType start = 0;
          page::IndexEntriesType end = indexEntries;

          while (start < end)
          {
            page::IndexEntriesType middle = start+(end-start)/2;

#ifdef DEBUG
            assert(middle < end);
#endif

            char* deltaPtr = startOfUncompressedSection + indexPtr[middle];
#ifdef DEBUG
            assert(page::readHeader(deltaPtr) == page::Header::StartOfDelta);
#else
            page::advance<HeaderType>(deltaPtr);
#endif
            page::advance<IdType>(deltaPtr);

            PrefixSizeType deltaPrefixSize = page::read<PrefixSizeType>(deltaPtr);
            if (prefixSize < deltaPrefixSize) {
              // The delta has a bigger matching prefix and is thus
              // lexicographically smaller -> abort early
              start = middle + 1;
              if (uncompressedGoodPtr != nullptr) {
                lastGoodPtr = startOfUncompressedSection + indexPtr[middle];
              }
            }
            else {
              // Compare delta string
              StringSizeType deltaSize = page::read<StringSizeType>(deltaPtr);
              if (prefixSize == str.size() && prefixSize != deltaPrefixSize) {
                break;
              }
              const char* delta = page::readString(deltaPtr, deltaSize);
              int cmp = memcmp(delta, str.c_str()+deltaPrefixSize, min<uint64_t>(deltaSize, str.size()-prefixSize));
              if (cmp == 0) {
                // Found a matching entry; go find the last one!
                lastGoodPtr = startOfUncompressedSection + indexPtr[middle];
                start = middle + 1;
              }
              else if (cmp < 0) {
                start = middle + 1;
              }
              else {
                end = middle;
              }
            }
          }
        }

        this->dataPtr = (lastGoodPtr == nullptr ? uncompressedGoodPtr : lastGoodPtr);
        return *this;
      }

      Iterator& prefixStartIndexSearch(const std::string& str) {
        assert(this->dataPtr != nullptr);

        assert(page::readHeader(this->dataPtr) == page::Header::StartOfIndex);
        page::IndexEntriesType indexEntries = page::read<page::IndexEntriesType>(this->dataPtr);

        OffsetType* indexPtr = reinterpret_cast<OffsetType*>(this->dataPtr);
        page::advance<OffsetType>(this->dataPtr, indexEntries);
        char* startOfUncompressedSection = this->dataPtr;

        assert(page::readHeader(this->dataPtr) == page::Header::StartOfUncompressedValue);
        page::advance<IdType>(this->dataPtr);
        StringSizeType fullStringSize = page::read<StringSizeType>(this->dataPtr);
        startOfFullString = this->dataPtr;
        const char* fullString = page::readString(this->dataPtr, fullStringSize);
        PrefixSizeType prefixSize = prefixLength(fullString, fullStringSize, str);

        if (prefixSize == str.size()) {
          this->dataPtr = startOfUncompressedSection;
          return *this;
        }

        if (indexEntries > 0) {
          // Perform binary search
          page::IndexEntriesType start = 0;
          page::IndexEntriesType end = indexEntries;

          while (start < end)
          {
            page::IndexEntriesType middle = start+(end-start)/2;

#ifdef DEBUG
            assert(middle < end);
#endif

            char* deltaPtr = startOfUncompressedSection + indexPtr[middle];
#ifdef DEBUG
            assert(page::readHeader(deltaPtr) == page::Header::StartOfDelta);
#else
            page::advance<HeaderType>(deltaPtr);
#endif
            page::advance<IdType>(deltaPtr);

            PrefixSizeType deltaPrefixSize = page::read<PrefixSizeType>(deltaPtr);
            if (prefixSize < deltaPrefixSize) {
              // The delta has a bigger matching prefix and is thus
              // lexicographically smaller -> abort early
              start = middle + 1;
            }
            else {
              // Compare delta string
              StringSizeType deltaSize = page::read<StringSizeType>(deltaPtr);
              const char* delta = page::readString(deltaPtr, deltaSize);

              int cmp = memcmp(delta, str.c_str()+deltaPrefixSize, min<uint64_t>(deltaSize, str.size()-prefixSize));
              if (cmp == 0) {
                this->dataPtr = startOfUncompressedSection + indexPtr[middle];
                return *this;
              }
              end = middle;
            }
          }
        }

        // Doesn't match
        this->dataPtr = nullptr;
        return *this;
      }

      Iterator& last() {
        assert(this->dataPtr != nullptr);

        assert(page::readHeader(this->dataPtr) == page::Header::StartOfIndex);

        page::IndexEntriesType indexEntries = page::read<page::IndexEntriesType>(this->dataPtr);

        if (indexEntries == 0) {
          // Return uncompressed value
          page::advance<OffsetType>(this->dataPtr, indexEntries);
          assert(page::readHeader(this->dataPtr) == page::Header::StartOfUncompressedValue);
          return *this;
        }

        OffsetType* indexPtr = reinterpret_cast<OffsetType*>(this->dataPtr);
        page::advance<OffsetType>(this->dataPtr, indexEntries);
        char* startOfUncompressedSection = this->dataPtr;

        char* readPtr = this->dataPtr;
        assert(readHeader(readPtr) == page::Header::StartOfUncompressedValue);
        advance<IdType>(readPtr);
        advance<StringSizeType>(readPtr);
        startOfFullString = readPtr;

        this->dataPtr = startOfUncompressedSection + indexPtr[indexEntries-1];
        return *this;
      }

      Iterator& getIndexEntry(page::IndexEntriesType entry) {
        assert(this->dataPtr != nullptr);
        assert(page::readHeader(this->dataPtr) == page::Header::StartOfIndex);
        page::IndexEntriesType indexEntries = page::read<page::IndexEntriesType>(this->dataPtr);
        assert(entry <= indexEntries);
        OffsetType* indexPtr = reinterpret_cast<OffsetType*>(this->dataPtr);

        page::advance<OffsetType>(this->dataPtr, indexEntries);
        char* startOfUncompressedSection = this->dataPtr;
        assert(page::readHeader(this->dataPtr) == page::Header::StartOfUncompressedValue);

        page::advance<IdType>(this->dataPtr);
        page::advance<StringSizeType>(this->dataPtr);
        startOfFullString = this->dataPtr;

        // entry = 0 means the uncompressed string
        if (entry == 0) {
          this->dataPtr = startOfUncompressedSection;
        }
        else {
          this->dataPtr = startOfUncompressedSection + indexPtr[entry-1];
        }
        return *this;
      }

      void debug() const {
        assert(currentPage != nullptr);
        char* readPtr = currentPage->getData();

        HeaderType header;
        const char* uncompressedStringPtr = nullptr;

        while (true) {
          header = readHeader(readPtr);
          if (header == Header::EndOfPage) {
            std::cout << "> End of page" << std::endl;
            break;
          }

          if (header == Header::StartOfIndex) {
            std::cout << "> Index section" << std::endl;
            page::IndexEntriesType indexEntries = read<page::IndexEntriesType>(readPtr);
            std::cout << "  " << indexEntries << " entries" << std::endl;
            advance<OffsetType>(readPtr, indexEntries);
            continue;
          }

          if (header == Header::StartOfUncompressedValue) {
            std::cout << "> Uncompressed section" << std::endl;
            IdType id = read<IdType>(readPtr);
            std::cout << "  " << id << std::endl;
            StringSizeType size = read<StringSizeType>(readPtr);
            uncompressedStringPtr = page::readString(readPtr, size);
            std::cout << "  (" << size << ") " << std::string(uncompressedStringPtr, size) << std::endl;
            continue;
          }

          if (header == Header::StartOfDelta) {
            std::cout << "> Compressed section" << std::endl;
            assert(uncompressedStringPtr != nullptr);
            IdType id = read<IdType>(readPtr);
            std::cout << "  " << id << std::endl;
            PrefixSizeType prefixSize = read<PrefixSizeType>(readPtr);
            StringSizeType stringSize = read<StringSizeType>(readPtr);
            const char* value = page::readString(readPtr, stringSize);
            std::cout << "  (" << stringSize << ") " << std::string(uncompressedStringPtr, prefixSize) + std::string(value, stringSize) << std::endl;
            continue;
          }

          throw;
        }
      }

      inline void gotoNextPage() {
        assert(nextPage != nullptr);
        this->currentPage = this->nextPage;
        this->nextPage = this->nextPage->nextPage;
        this->dataPtr = this->currentPage->getData();
        startOfFullString = nullptr;
      }

      Iterator& indexSearch(const std::string& str) {
        assert(this->dataPtr != nullptr);

        assert(page::readHeader(this->dataPtr) == page::Header::StartOfIndex);

        page::IndexEntriesType indexEntries = page::read<page::IndexEntriesType>(this->dataPtr);

        OffsetType* indexPtr = reinterpret_cast<OffsetType*>(this->dataPtr);
        page::advance<OffsetType>(this->dataPtr, indexEntries);
        char* startOfUncompressedSection = this->dataPtr;

        assert(page::readHeader(this->dataPtr) == page::Header::StartOfUncompressedValue);
        page::advance<IdType>(this->dataPtr);
        StringSizeType fullStringSize = page::read<StringSizeType>(this->dataPtr);
        startOfFullString = this->dataPtr;
        const char* fullString = page::readString(this->dataPtr, fullStringSize);
        PrefixSizeType prefixSize = prefixLength(fullString, fullStringSize, str);

        if (prefixSize == str.size() && fullStringSize == prefixSize) {
          this->dataPtr = startOfUncompressedSection;
          return *this;
        }

        if (indexEntries > 0) {
          // Perform binary search
          page::IndexEntriesType start = 0;
          page::IndexEntriesType end = indexEntries;

          // First, check if this is even the correct page
          char* readPtr = startOfUncompressedSection+indexPtr[indexEntries-1];
          assert(page::readHeader(readPtr) == page::Header::StartOfDelta);
          page::advance<IdType>(readPtr);
          PrefixSizeType endPrefixSize = page::read<PrefixSizeType>(readPtr);
          if (prefixSize < endPrefixSize) {
            // The delta has a bigger matching prefix and is thus
            // lexicographically smaller -> goto next page
            gotoNextPage();
            return indexSearch(str);
          }
          else {
            // Compare delta string
            StringSizeType endSize = page::read<StringSizeType>(readPtr);
            const char* delta = page::readString(readPtr, endSize);
            int cmp = memcmp(delta, &str.c_str()[endPrefixSize], min<uint64_t>(str.size()-endPrefixSize, endSize));
            if (cmp == 0) {
              if (str.size() == endSize+endPrefixSize) {
                this->dataPtr = startOfUncompressedSection + indexPtr[indexEntries-1];
                return *this;
              }
              if (str.size() > endSize+endPrefixSize) {
                gotoNextPage();
                return indexSearch(str);
              }
            }
            else if (cmp < 0) {
              gotoNextPage();
              return indexSearch(str);
            }
          }
          end--;

          while (start < end)
          {
            page::IndexEntriesType middle = start+(end-start)/2;

#ifdef DEBUG
            assert(middle < end);
#endif

            char* deltaPtr = startOfUncompressedSection + indexPtr[middle];

#ifdef DEBUG
            assert(page::readHeader(deltaPtr) == page::Header::StartOfDelta);
#else
            page::advance<HeaderType>(deltaPtr);
#endif
            page::advance<IdType>(deltaPtr);

            PrefixSizeType deltaPrefixSize = page::read<PrefixSizeType>(deltaPtr);
            if (prefixSize < deltaPrefixSize) {
              // The delta has a bigger matching prefix and is thus
              // lexicographically smaller -> abort early
              start = middle + 1;
            }
            else {
              // Compare delta string
              StringSizeType deltaSize = page::read<StringSizeType>(deltaPtr);
              const char* delta = page::readString(deltaPtr, deltaSize);
              int cmp = memcmp(delta, &str.c_str()[deltaPrefixSize], min<uint64_t>(str.size()-deltaPrefixSize, deltaSize));
              if (cmp == 0) {
                if (str.size() == deltaSize+deltaPrefixSize) {
                  this->dataPtr = startOfUncompressedSection + indexPtr[middle];
                  return *this;
                }
                else if (str.size() > deltaSize+deltaPrefixSize) {
                  start = middle+1;
                }
                else {
                  end = middle;
                }
              }
              else if (cmp < 0) {
                start = middle+1;
              }
              else {
                end = middle;
              }
            }
          }
        }

        // Doesn't match
        this->dataPtr = nullptr;
        return *this;
      }

      Iterator& find(IdType id) {
        assert(this->dataPtr != nullptr);
        do {
          char* readPtr = this->dataPtr;
#ifdef DEBUG
          auto hdr = page::readHeader(readPtr);
          assert(hdr  == page::Header::StartOfUncompressedValue);
#else
          page::advance<page::HeaderType>(readPtr);
#endif
          IdType foundId = page::read<IdType>(readPtr);
          if (id == foundId) {
            return *this;
          }

          page::Header header = page::readHeader(this->dataPtr);
          // Not found; skip according to header
          if (header == page::Header::StartOfUncompressedValue) {
            page::advance<IdType>(this->dataPtr);
            StringSizeType size = page::read<StringSizeType>(this->dataPtr);
            startOfFullString = this->dataPtr;
            page::advance(this->dataPtr, size);
          }
          else if (header == page::Header::StartOfDelta) {
            page::advance<IdType>(this->dataPtr);
            page::advance<PrefixSizeType>(this->dataPtr);
            StringSizeType size = page::read<StringSizeType>(this->dataPtr);
            page::advance(this->dataPtr, size);
          }

          readPtr = this->dataPtr;
          header = page::readHeader(readPtr);
          if (header == page::Header::StartOfUncompressedValue || header == page::Header::StartOfDelta) {
          }
          else if (header == page::Header::EndOfPage && this->nextPage != nullptr) {
            this->dataPtr = this->nextPage->getData();
            this->nextPage = this->nextPage->nextPage;
            this->startOfFullString = nullptr;
          }
          else {
            this->dataPtr = nullptr;
          }
        }
        while (this->dataPtr != nullptr);

        return *this;
      }

      Iterator& find(const std::string& searchValue) {
        assert(this->dataPtr != nullptr);
        uint64_t pos = 0;
        do {
          char* readPtr = this->dataPtr;
          page::Header header = page::readHeader(readPtr);
          page::advance<IdType>(readPtr);

          if (header == page::Header::StartOfUncompressedValue) {
            // String is stored uncompressed
            StringSizeType size = page::read<StringSizeType>(readPtr);
            const char* value = page::readString(readPtr, size);

            pos = 0;
            while(pos < size && pos < searchValue.size() && searchValue[pos] == value[pos]) {
              pos++;
            }

            if (pos == searchValue.size()) {
              // string found; return
              return *this;
            }
            this->fullString = std::string(value, value+size);
          }
          else if (header == page::Header::StartOfDelta) {
            PrefixSizeType prefixSize = page::read<PrefixSizeType>(readPtr);
            StringSizeType size = page::read<StringSizeType>(readPtr);
            if (prefixSize == pos && searchValue.size() <= prefixSize + size) {
              // Possible match; compare characters
              const char* value = page::readString(readPtr, size);

              uint64_t searchPos = 0;
              while(searchPos+pos < searchValue.size() && searchValue[pos+searchPos] == value[searchPos]) {
                searchPos++;
              }

              if (searchPos+pos == searchValue.size()) {
                // string found; return
                return *this;
              }
            }
          }

          header = page::readHeader(this->dataPtr);
          // Not found; skip according to header
          if (header == page::Header::StartOfUncompressedValue) {
            page::advance<IdType>(this->dataPtr);
            StringSizeType size = page::read<StringSizeType>(this->dataPtr);
            page::advance(this->dataPtr, size);
          }
          else if (header == page::Header::StartOfDelta) {
            page::advance<IdType>(this->dataPtr);
            page::advance<PrefixSizeType>(this->dataPtr);
            StringSizeType size = page::read<StringSizeType>(this->dataPtr);
            page::advance(this->dataPtr, size);
          }

          readPtr = this->dataPtr;
          header = page::readHeader(readPtr);
          if (header == page::Header::StartOfUncompressedValue || header == page::Header::StartOfDelta) {
          }
          else if (header == page::Header::EndOfPage && this->nextPage != nullptr) {
            this->dataPtr = this->nextPage->getData();
            this->nextPage = this->nextPage->nextPage;
            this->startOfFullString = nullptr;
          }
          else {
            this->dataPtr = nullptr;
          }
        }
        while (this->dataPtr != nullptr);

        return *this;
      }

      /*Iterator& findMax(const std::string& searchValue) {
        assert(this->dataPtr != nullptr);
        uint64_t pos = 0;

        char* lastFoundPtr = nullptr;
        TPage* lastFoundPage = nullptr;
        char* lastFoundStart = nullptr;

        do {
        char* readPtr = this->dataPtr;
        page::Header header = page::readHeader(readPtr);
        page::read<IdType>(readPtr);

        if (header == page::Header::StartOfUncompressedValue) {
      // String is stored uncompressed
      StringSizeType size = page::read<StringSizeType>(readPtr);
      startOfFullString = readPtr;
      const char* value = page::readString(readPtr, size);

      pos = 0;
      while(pos < size && pos < searchValue.size() && searchValue[pos] == value[pos]) {
      pos++;
      }

      if (pos == searchValue.size()) {
      // string found; skip to next
      lastFoundPtr = dataPtr;
      lastFoundPage = nextPage;
      goto skip;
      }
      else if (lastFoundPtr != nullptr) {
      // Return last found value
      dataPtr = lastFoundPtr;
      nextPage = lastFoundPage;
      startOfFullString = lastFoundStart;
      return *this;
      }
      }
      else if (header == page::Header::StartOfDelta) {
      PrefixSizeType prefixSize = page::read<PrefixSizeType>(readPtr);
      StringSizeType size = page::read<StringSizeType>(readPtr);
      if (prefixSize == pos && searchValue.size() <= prefixSize + size) {
      // Possible match; compare characters
      const char* value = page::readString(readPtr, size);

      uint64_t searchPos = 0;
      while(searchPos+pos < searchValue.size() && searchValue[pos+searchPos] == value[searchPos]) {
      searchPos++;
      }

      if (searchPos+pos == searchValue.size()) {
      // string found; skip to next
      lastFoundPtr = dataPtr;
      lastFoundPage = nextPage;
      goto skip;
      }
      else if (lastFoundPtr != nullptr) {
      // Return last found value
      dataPtr = lastFoundPtr;
      nextPage = lastFoundPage;
      return *this;
      }
      }
      }

skip:
header = page::readHeader(this->dataPtr);
      // Not found; skip according to header
      if (header == page::Header::StartOfUncompressedValue) {
      page::advance<IdType>(this->dataPtr);
      StringSizeType size = page::read<StringSizeType>(this->dataPtr);
      page::advance(this->dataPtr, size);
    }
      else if (header == page::Header::StartOfDelta) {
        page::advance<IdType>(this->dataPtr);
        page::advance<PrefixSizeType>(this->dataPtr);
        StringSizeType size = page::read<StringSizeType>(this->dataPtr);
        page::advance(this->dataPtr, size);
      }

      readPtr = this->dataPtr;
      header = page::readHeader(readPtr);
      if (header == page::Header::StartOfUncompressedValue || header == page::Header::StartOfDelta) {
      }
      else if (header == page::Header::EndOfPage && this->nextPage != nullptr) {
        this->dataPtr = this->nextPage->getData();
        this->nextPage = this->nextPage->nextPage;
      }
      else {
        this->dataPtr = nullptr;
      }
    }
        while (this->dataPtr != nullptr);

      if (lastFoundPtr != nullptr) {
        // rewind
        dataPtr = lastFoundPtr;
        nextPage = lastFoundPage;
      }

      return *this;
    }*/

    Iterator& gotoOffsetWithDelta(uint16_t offset, uint16_t delta) {
      assert(this->dataPtr != nullptr);
      this->dataPtr += offset;

      uint16_t pos = 0;
      while (pos < delta) {
        page::Header header = page::readHeader(this->dataPtr);
        // Skip according to header
        if (header == page::Header::StartOfUncompressedValue) {
          page::advance<IdType>(this->dataPtr);
          StringSizeType size = page::read<StringSizeType>(this->dataPtr);
          this->startOfFullString = this->dataPtr;
          page::advance(this->dataPtr, size);
        }
        else if (header == page::Header::StartOfDelta) {
          page::advance<IdType>(this->dataPtr);
          page::advance<PrefixSizeType>(this->dataPtr);
          StringSizeType size = page::read<StringSizeType>(this->dataPtr);
          page::advance(this->dataPtr, size);
        }
        else {
          throw Exception("Read unknown header");
        }

        pos++;
      }

      return *this;
    }

    Iterator& gotoIndexOffset(uint16_t offset) {
      assert(this->dataPtr != nullptr);

      assert(page::readHeader(dataPtr) == page::Header::StartOfIndex);
      page::IndexEntriesType indexEntries = page::read<page::IndexEntriesType>(dataPtr);
      page::advance<OffsetType>(dataPtr, indexEntries);

      if (offset == 0) {
        return *this;
      }

      char* offsetPtr = this->dataPtr+offset;
      page::Header header = page::readHeader(this->dataPtr);
      assert(header == page::Header::StartOfUncompressedValue);

      // Read full string
      page::advance<IdType>(this->dataPtr);
      page::advance<StringSizeType>(this->dataPtr);
      startOfFullString = this->dataPtr;

      // Set pointer to delta
      this->dataPtr = offsetPtr;
      header = page::readHeader(offsetPtr);
      assert(header == page::Header::StartOfDelta);

      return *this;
    }

    Iterator& gotoOffset(uint16_t offset) {
      assert(this->dataPtr != nullptr);
      if (offset == 0) {
        return *this;
      }

      char* offsetPtr = this->dataPtr+offset;
      page::Header header = page::readHeader(this->dataPtr);
      assert(header == page::Header::StartOfUncompressedValue);

      // Read full string
      page::advance<IdType>(this->dataPtr);
      page::advance<StringSizeType>(this->dataPtr);
      startOfFullString = this->dataPtr;

      // Set pointer to delta
      this->dataPtr = offsetPtr;
      header = page::readHeader(offsetPtr);
      assert(header == page::Header::StartOfDelta);

      return *this;
    }

    Iterator& gotoDelta(uint16_t delta) {
      assert(this->dataPtr != nullptr);
      uint16_t pos = 0;
      while (pos < delta) {
        page::Header header = page::readHeader(this->dataPtr);
        // Not found; skip according to header
        if (header == page::Header::StartOfUncompressedValue) {
          page::advance<IdType>(this->dataPtr);
          StringSizeType size = page::read<StringSizeType>(this->dataPtr);
          startOfFullString = this->dataPtr;
          page::advance(this->dataPtr, size);
        }
        else if (header == page::Header::StartOfDelta) {
          page::advance<IdType>(this->dataPtr);
          page::advance<PrefixSizeType>(this->dataPtr);
          StringSizeType size = page::read<StringSizeType>(this->dataPtr);
          page::advance(this->dataPtr, size);
        }

        pos++;
      }

      char* readPtr = this->dataPtr;
      page::Header header = page::readHeader(readPtr);
      if (header == page::Header::StartOfUncompressedValue || header == page::Header::StartOfDelta) {
      }
      else if (header == page::Header::EndOfPage && this->nextPage != nullptr) {
        this->dataPtr = this->nextPage->getData();
        this->nextPage = this->nextPage->nextPage;
        this->startOfFullString = nullptr;
      }
      else {
        this->dataPtr = nullptr;
      }

      return *this;
    }
    };

  template<uint64_t TSize, class TPage>
    class Page {
      public:
        const uint64_t size = TSize;
        TPage* nextPage;
        char data[TSize];

        inline char* getData() {
          return data;
        }

        Page() : nextPage(nullptr) {
        }
    };
}

template<uint64_t TSize, class TPage>
using Page = page::Page<TSize, TPage>;

template<class TPage>
using PageLoader = page::Loader<TPage>;

template<class TPage>
using PageIterator = page::Iterator<TPage>;

#endif
