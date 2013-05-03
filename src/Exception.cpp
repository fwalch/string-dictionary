#include "Exception.hpp"

Exception::Exception(const std::string msg) : message(msg) { }

const char* Exception::what() const noexcept {
  return message.c_str();
}
