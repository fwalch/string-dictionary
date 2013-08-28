#include "Dictionary.hpp"

Dictionary::~Dictionary() noexcept {
}

uint64_t Dictionary::size() const {
  return nextId;
}

std::ostream& operator<<(std::ostream &stream, const Dictionary* dict) {
  return stream << dict->description();
}
