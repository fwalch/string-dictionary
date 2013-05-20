#include <sstream>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "TurtleParser.hpp"
#include "mocks/DictionaryLoaderMock.hpp"

using namespace std;
using ::testing::_;

// TODO: tests contain no newlines; parser only reads lines
// Re-enable tests

TEST(TurtleParser, DISABLED_Comment) {
  stringstream stream("# comments should be ignored");
  TurtleParser parser(stream);

  DictionaryLoaderMock mock;
  EXPECT_CALL(mock, setBasePrefix(_)).Times(0);
  EXPECT_CALL(mock, addPrefix(_, _)).Times(0);
  EXPECT_CALL(mock, addString(_, _)).Times(0);

  parser.loadInto(mock);
}

TEST(TurtleParser, DISABLED_BasePrefix) {
  stringstream stream("@base <http://base/prefix#test> .");
  TurtleParser parser(stream);

  DictionaryLoaderMock mock;
  EXPECT_CALL(mock, setBasePrefix(_)).Times(0);
  EXPECT_CALL(mock, setBasePrefix("http://base/prefix#test"));
  EXPECT_CALL(mock, addPrefix(_, _)).Times(0);
  EXPECT_CALL(mock, addString(_, _)).Times(0);

  parser.loadInto(mock);
}

TEST(TurtleParser, DISABLED_Prefix) {
  stringstream stream("@prefix some: <http://prefix#test> .");
  TurtleParser parser(stream);

  DictionaryLoaderMock mock;
  EXPECT_CALL(mock, setBasePrefix(_)).Times(0);
  EXPECT_CALL(mock, addPrefix(_, _)).Times(0);
  EXPECT_CALL(mock, addPrefix("some", "http://prefix#test"));
  EXPECT_CALL(mock, addString(_, _)).Times(0);

  parser.loadInto(mock);
}

TEST(TurtleParser, DISABLED_Triple) {
  stringstream stream("<Subject> <Predicate> <Object> .");
  TurtleParser parser(stream);

  DictionaryLoaderMock mock;
  EXPECT_CALL(mock, setBasePrefix(_)).Times(0);
  EXPECT_CALL(mock, addPrefix(_, _)).Times(0);
  EXPECT_CALL(mock, addString(_, _)).Times(0);

  EXPECT_CALL(mock, addString("", "Subject"));
  EXPECT_CALL(mock, addString("", "Predicate"));
  EXPECT_CALL(mock, addString("", "Object"));

  parser.loadInto(mock);
}

TEST(TurtleParser, DISABLED_PrefixedTriples) {
  stringstream stream(R"(
    @base <http://base#prefix>
    @prefix a: <http://second#prefix>
    @prefix b: <http://third#prefix>
    <Subject> a:Predicate b:Object .
  )");
  TurtleParser parser(stream);

  DictionaryLoaderMock mock;
  EXPECT_CALL(mock, setBasePrefix(_)).Times(0);
  EXPECT_CALL(mock, setBasePrefix("http://base#prefix"));
  EXPECT_CALL(mock, addPrefix(_, _)).Times(0);
  EXPECT_CALL(mock, addPrefix("a", "http://second#prefix"));

  EXPECT_CALL(mock, addString("http://base#prefix", "Subject"));
  EXPECT_CALL(mock, addString("a", "Predicate"));
  EXPECT_CALL(mock, addString("b", "Object"));

  parser.loadInto(mock);
}
