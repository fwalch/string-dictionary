gtest_sources = src/gtest.cpp src/gtest-death-test.cpp \
								src/gtest-filepath.cpp src/gtest-port.cpp \
								src/gtest-printers.cpp src/gtest-test-part.cpp  \
								src/gtest-typed-test.cpp
gtest_cxxflags = -Wno-everything -Igtest
gtest_ldflags = -lpthread
