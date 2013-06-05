gmock_sources = src/gmock-cardinalities.cpp src/gmock.cpp \
								src/gmock-internal-utils.cpp src/gmock-matchers.cpp \
								src/gmock-spec-builders.cpp
gmock_cxxflags = -w -Igmock -Igmock/include
gmock_libraries = gtest
