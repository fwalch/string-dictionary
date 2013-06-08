#ifndef H_PerformanceTestRunner
#define H_PerformanceTestRunner

#include <istream>
#include "DummyDictionary.hpp"

class PerformanceTestRunner {
  public:
    void run(std::istream& tupleStream);
};

#endif
