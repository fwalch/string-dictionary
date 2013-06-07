#ifndef H_TurtleParser
#define H_TurtleParser

#include <istream>
#include "Dictionary.hpp"

/**
 * Parses Turtle data from an input stream.
 */
class TurtleParser {
  private:
    std::istream& input;

  public:
    TurtleParser(std::istream& input);
    void loadInto(Dictionary* dictionary);
};

#endif
