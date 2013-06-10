test_sources = UncompressedDictionaryTests.cpp SimpleDictionaryTests.cpp \
							 DummyDictionaryTests.cpp TurtleParserTests.cpp \
							 AdaptiveRadixTreeTests.cpp ARTDictionaryTests.cpp \
							 HashARTDictionaryTests.cpp BTreeDictionaryTests.cpp \
							 BPlusTreeDictionaryTests.cpp
test_executables = test
test_dependencies = src
test_libraries = gmock gtest btree b+tree boost
test_cxxflags = -Wno-global-constructors -Wno-c++98-compat-pedantic -Wno-weak-vtables
