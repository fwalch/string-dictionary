#include "Dictionary.hpp"

Dictionary::~Dictionary() noexcept {
}

Dictionary::IdType Dictionary::size() const {
  return nextId;
}
