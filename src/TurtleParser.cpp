#include <cassert>
#include <istream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include "Exception.hpp"
#include "TurtleParser.hpp"
#include "boost/algorithm/string.hpp"

using namespace std;

TurtleParser::TurtleParser(istream& inputStream) : input(inputStream) { }

//TODO: more robust parser
void TurtleParser::loadInto(Dictionary* dictionary) {
  if (!input.good()) {
    throw Exception("Could not read from stream");
  }

  string BASE("@base");
  string PREFIX("@prefix");

  string basePrefix;
  unordered_map<string, string> prefixes;

  string line;
  while (getline(input, line).good()) {
    if (line.empty() || line.at(0) == '#') continue;

    if (line.at(0) == '@') {
      // Prefix
      if (boost::starts_with(line, BASE)) {
        assert(basePrefix.empty());
        basePrefix = string(line.begin() + static_cast<long>(BASE.size()) + 2, line.end() - 3) + "/";
      }
      else {
        long pos = static_cast<long>(line.find_first_of(":"));
        string prefixName = string(line.begin() + static_cast<long>(PREFIX.size()) + 1, line.begin() + pos);
        string prefix = string(line.begin() + pos + 3, line.end() - 3);
        prefixes.insert(make_pair(prefixName, prefix + "/"));
      }
      continue;
    }

    stringstream ss(line);
    string elem;

    while (getline(ss, elem, '\t').good()) {
      if (elem.at(0) == '<') {
        dictionary->insert(basePrefix + string(elem.begin() + 1, elem.end() - 1));
      }
      else {
        long pos = static_cast<long>(elem.find(":"));
        string prefixName = string(elem.begin(), elem.begin() + pos);
        dictionary->insert(prefixes[prefixName] + string(elem.begin() + pos + 1, elem.end()));
      }
    }
    if (elem.at(0) == '<') {
      dictionary->insert(basePrefix + string(elem.begin() + 1, elem.end() - 3));
    }
    else {
      long pos = static_cast<long>(elem.find(":"));
      string prefixName = string(elem.begin(), elem.begin() + pos);
      dictionary->insert(prefixes[prefixName] + string(elem.begin() + pos + 1, elem.end() - 2));
    }
  }
}
