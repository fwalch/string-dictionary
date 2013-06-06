#include "Dictionary.hpp"

Dictionary::~Dictionary() noexcept {
}

uint64_t Dictionary::size() const {
  return nextId;
}
