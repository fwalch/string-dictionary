#include "Exception.hpp"

Exception::Exception(const std::string msg) : msg(msg) { }

const char* Exception::what() const noexcept {
  return msg.c_str();
}
