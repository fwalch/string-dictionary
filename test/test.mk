test_sources = UncompressedDictionaryTests.cpp SimpleDictionaryTests.cpp \
							 DummyDictionaryTests.cpp TurtleParserTests.cpp \
							 AdaptiveRadixTreeTests.cpp ARTDictionaryTests.cpp
test_executables = test
test_dependencies = src gtest gmock
# TODO: not ideal, but googletest gives so many warnings, even with when included with -isystem...
test_cxxflags = -w
