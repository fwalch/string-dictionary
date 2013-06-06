#include <algorithm>
#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <unordered_set>
#include "boost/filesystem.hpp"
#include "boost/algorithm/string.hpp"

/**
 * @file
 * Splits a turtle file in two files according to a ratio.
 */

static constexpr const char* LargeFilePostfix = "-large.";
static constexpr const char* SmallFilePostfix = "-small.";

inline int usageMessage(const char* argv0) {
  std::cerr << "Usage: " << argv0 << " [ratio] [turtle file]" << std::endl
            << "ratio must be a floating point number out of ]0,1[." << std::endl;
  return 1;
}

inline bool skipLine(std::string line) {
  return line.empty() || line[0] == '#';
}

int main(int argc, const char** argv) {
  if (argc != 3) {
    return usageMessage(argv[0]);
  }

  // Verify that the ratio is valid
  float ratio;
  std::stringstream ss(argv[1]);
  try {
    ss >> ratio;
    if (ratio <= 0 || ratio >= 1) {
      return usageMessage(argv[0]);
    }
    if (ratio > 0.5) {
      ratio = 1-ratio;
    }
  }
  catch (...) {
    return usageMessage(argv[0]);
  }

  // Verify that file name is valid
  const char* fileName = argv[2];
  std::ifstream file(fileName);
  if (!file.good()) {
    return usageMessage(argv[0]);
  }

  auto path = boost::filesystem::path(fileName);
  auto extension = path.extension();
  auto filename = path.filename();
  auto dir = path.remove_filename();
  dir /= filename.replace_extension();

  auto largePath = boost::filesystem::path(dir);
  largePath += LargeFilePostfix;
  largePath.replace_extension(extension);

  auto smallPath = boost::filesystem::path(dir);
  smallPath += SmallFilePostfix;
  smallPath.replace_extension(extension);

  std::string largeFileName = largePath.string();
  std::string smallFileName = smallPath.string();

  std::cout << "Splitting Turtle data from '" << fileName << "' into two files:" << std::endl;
  std::cout << " > " << largeFileName << " (ratio " << 1-ratio << ")" << std::endl;
  std::cout << " > " << smallFileName << " (ratio " << ratio << ")" << std::endl;

  uint64_t offset = 0;
  std::string line;
  while (std::getline(file, line).good()
      && !(boost::starts_with(line, "#@ <id_") || (!line.empty() && line[0] == '<'))) {
    offset++;
  }

  uint64_t lineCount = 0;
  while (std::getline(file, line).good()) {
    if (!skipLine(line)) {
      lineCount++;
    }
  }

  uint64_t smallFileTuples = static_cast<uint64_t>(ratio*lineCount);
  std::unordered_set<uint64_t> lineNumbers(static_cast<size_t>(smallFileTuples));

  std::random_device device;
  std::mt19937_64 engine(device());
  std::uniform_int_distribution<uint64_t> dist(0, lineCount-1);

  uint64_t i = 1;
  while (i <= smallFileTuples) {
    lineNumbers.insert(dist(engine));
    if (lineNumbers.size() == static_cast<size_t>(i)) {
      i++;
    }
  }
  assert(lineNumbers.size() == static_cast<size_t>(smallFileTuples));

  std::ofstream smallFile(smallFileName);
  std::ofstream largeFile(largeFileName);

  file.clear();
  file.seekg(0, file.beg);

  for (i = 0; i < offset; i++) {
    assert(std::getline(file, line).good());
    smallFile << line << std::endl;
    largeFile << line << std::endl;
  }

  i = 0;
  while(std::getline(file, line).good()) {
    if (skipLine(line)) {
      continue;
    }

    if (lineNumbers.find(i) != lineNumbers.end()) {
      smallFile << line << std::endl;
    }
    else {
      largeFile << line << std::endl;
    }

    i++;
  }

  smallFile.close();
  largeFile.close();
  file.close();

  assert(i == lineCount);

  std::cout << "Done." << std::endl;

  return 0;
}
