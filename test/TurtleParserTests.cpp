#include <sstream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TurtleParser.hpp"
#include "mocks/DictionaryMock.hpp"

using namespace std;
using ::testing::_;

TEST(TurtleParser, Comment) {
  stringstream stream("# comments should be ignored\n");
  TurtleParser parser(stream);

  DictionaryMock mock;
  EXPECT_CALL(mock, insert(_)).Times(0);

  parser.loadInto(&mock);
}

TEST(TurtleParser, BasePrefix) {
  stringstream stream("@base <http://base/prefix#test/> .\n<A>\t<B>\t<C> .\n");
  TurtleParser parser(stream);

  DictionaryMock mock;
  EXPECT_CALL(mock, insert("http://base/prefix#test/A"));
  EXPECT_CALL(mock, insert("http://base/prefix#test/B"));
  EXPECT_CALL(mock, insert("http://base/prefix#test/C"));

  parser.loadInto(&mock);
}

TEST(TurtleParser, Prefix) {
  stringstream stream("@prefix some: <http://prefix#test/> .\n<A>\tsome:B\t<C> .\n");
  TurtleParser parser(stream);

  DictionaryMock mock;
  EXPECT_CALL(mock, insert("A"));
  EXPECT_CALL(mock, insert("http://prefix#test/B"));
  EXPECT_CALL(mock, insert("C"));

  parser.loadInto(&mock);
}

TEST(TurtleParser, MultiplePrefixes) {
  stringstream stream("@base <http://base#prefix/> .\n@prefix a: <http://second#prefix/> .\n@prefix b: <http://third#prefix/> .\n<Subject>\ta:Predicate\tb:Object .\n<Subject>\ta:Predicate\t<Object> .\n");
  TurtleParser parser(stream);

  DictionaryMock mock;
  EXPECT_CALL(mock, insert("http://base#prefix/Subject")).Times(2);
  EXPECT_CALL(mock, insert("http://second#prefix/Predicate")).Times(2);
  EXPECT_CALL(mock, insert("http://third#prefix/Object"));
  EXPECT_CALL(mock, insert("http://base#prefix/Object"));

  parser.loadInto(&mock);
}
