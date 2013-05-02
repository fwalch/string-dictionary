#ifndef H_Exception
#define H_Exception

#include <exception>
#include <string>

class Exception : public std::exception {
  private:
    const std::string msg;

  public:
    Exception(const std::string message);
    const char* what() const noexcept;
};

#endif
