#include "Exception.hpp"
#include "TurtleParser.hpp"
#include <istream>
#include <sstream>
#include <iostream>

using namespace std;

TurtleParser::TurtleParser(istream& inputStream) : input(inputStream) { }

void TurtleParser::loadInto(DictionaryLoader& loader) {
  if (!input.good()) {
    throw Exception("Could not read from stream");
  }

  string line;
  while (getline(input, line).good()) {
    if (line.empty() || line.at(0) == '#') continue;

    if (line.at(0) == '@') {
      // Prefix
      string base = "@base";
      string prefix = "@prefix";
      if (line.compare(0, base.size(), base) == 0) {
        string value = string(line.begin() + static_cast<long>(base.size()) + 2, line.end() - 3);
        loader.setBasePrefix(value);
      }
      else {
        long pos = static_cast<long>(line.find_first_of(":"));
        string prefixValue = string(line.begin() + static_cast<long>(prefix.size()) + 1, line.begin() + pos);
        string value = string(line.begin() + pos + 3, line.end() - 3);
        loader.addPrefix(prefixValue, value);
      }
      continue;
    }

    stringstream ss(line);
    string elem;

    while (getline(ss, elem, '\t').good()) {
      if (elem.at(0) == '<') {
        loader.addString("", string(elem.begin() + 1, elem.end() - 1));
      }
      else {
        long pos = static_cast<long>(elem.find(":"));
        loader.addString(
            string(elem.begin(), elem.begin() + pos),
            string(elem.begin() + pos, elem.end()));
      }
    }
  }
}
