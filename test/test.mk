test_sources = UncompressedDictionaryTests.cpp SimpleDictionaryTests.cpp \
							 DummyDictionaryTests.cpp
test_executables = test
test_dependencies = src gtest
test_cxxflags = -Wno-everything
