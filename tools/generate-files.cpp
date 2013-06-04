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

  int64_t lineCount = 0;
  int64_t offset = 0;
  std::string line;
  while (std::getline(file, line).good()) {
    if (line.empty()) {
      continue;
    }
    lineCount++;
    //TODO
    if (offset == 0 && boost::starts_with(line, "#@ <id_")) {
      offset = lineCount;
      lineCount = 1;
    }
  }

  if (lineCount < 0) {
    std::cerr << "File " << fileName << " is too large." << std::endl;
    return 2;
  }

  assert(lineCount%2 == 0);
  lineCount /= 2;

  int64_t smallFileTuples = static_cast<int64_t>(ratio*lineCount);
  std::unordered_set<int64_t> lineNumbers(static_cast<size_t>(smallFileTuples));

  std::random_device device;
  std::mt19937_64 engine(device());
  std::uniform_int_distribution<int64_t> dist(0, lineCount);

  int64_t i = 1;
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

  i = 0;
  while (i < offset) {
    assert(std::getline(file, line).good());
    smallFile << line << std::endl;
    largeFile << line << std::endl;

    if (!line.empty()) {
      i++;
    }
  }

  i = 1;
  while (std::getline(file, line).good()) {
    if (lineNumbers.find(i/2) != lineNumbers.end()) {
      smallFile << line << std::endl;
    }
    else {
      largeFile << line << std::endl;
    }

    if (!line.empty()) {
      i++;
    }
  }

  smallFile.close();
  largeFile.close();
  file.close();

  assert(i == 2*lineCount);

  std::cout << "Done." << std::endl;

  return 0;
}
