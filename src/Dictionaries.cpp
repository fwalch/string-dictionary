#include "Dictionaries.hpp"

std::ostream& operator<<(std::ostream &stream, const Dictionary* dict) {
  return stream << dict->name();
}
