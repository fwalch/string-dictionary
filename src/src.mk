src_sources = Exception.cpp TurtleParser.cpp Dictionary.cpp DummyDictionary.cpp UncompressedDictionary.cpp \
							SimpleDictionary.cpp ARTDictionary.cpp AdaptiveRadixTree.cpp IndexART.cpp ReverseIndexART.cpp \
							HashARTDictionary.cpp BTreeDictionary.cpp PerformanceTestRunner.cpp Dictionaries.cpp \
							BPlusTreeDictionary.cpp ARTcDictionary.cpp HATDictionary.cpp \
							MARTDictionary.cpp ReverseIndexMART.cpp
src_executables = load perftest
src_libraries = btree b+tree boost libart hat
src_cxxflags=-Wno-unused-variable -Wno-unused-parameter
