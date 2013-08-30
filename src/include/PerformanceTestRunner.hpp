#ifndef H_PerformanceTestRunner
#define H_PerformanceTestRunner

#include <istream>

class PerformanceTestRunner {
  public:
    void run(std::istream& tupleStream);
};

class MicroTestRunner {
  public:
    void run(std::istream& tupleStream);
};

#endif
