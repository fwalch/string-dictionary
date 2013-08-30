test_sources = PageTests.cpp ARTTests.cpp HATTests.cpp
test_executables = test
test_dependencies = src
test_libraries = gmock gtest
test_cxxflags = -Wno-global-constructors -Wno-c++98-compat-pedantic -Wno-weak-vtables -Wno-unused-parameter -Wno-unused-variable

test: compile-test
	$(EXE_DIR)/test

build: clean debug-test
	valgrind --error-exitcode=2 $(EXE_DIR)/test
