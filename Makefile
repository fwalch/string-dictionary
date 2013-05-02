# Redefine CXX only if not passed from environment variable
ifeq ($(origin CXX), default)
	ifneq ($(shell which clang++ 2>/dev/null),)
		CXX = clang++
	else
		CXX = g++
	endif
endif

# Define clang flags
ifeq ($(CXX), clang++)
	CXXFLAGS = -g -Weverything -Werror -Wno-shadow -Wno-c++98-compat --std=c++11
	LDFLAGS =
endif

# Define gcc flags
ifeq ($(CXX), g++)
	CXXFLAGS = -g -Wall -Wextra -Werror --std=c++11
	LDFLAGS =
endif

# Define build types
ifdef release
	CXXFLAGS += -O3
	LDFLAGS +=
else
	CXXFLAGS += -O0
	LDFLAGS +=
endif

# Configuration variables
OBJ_DIR = obj
EXE_DIR = bin

CHECKDIR = mkdir -p $$(dir $$@)
BUILDOBJ = $(CXX) -c $(CXXFLAGS) -MD -MF $$(@:%.o=%.d) $$< -o $$@
BUILDEXE = $(CXX) $(LDFLAGS) $$^ -o $$@

# Main targets
all: compile-load
clean:
	find $(OBJ_DIR) -type f -and -name '*.o' -delete 2> /dev/null || true
	find $(EXE_DIR) -type f -and -not -name '*.sh' -delete 2> /dev/null || true
purge: clean
	rm -rf $(OBJ_DIR)

# Generate targets for source trees
#
# Will also generate a "compile-%" and
# a "%" target for compiling and running
# an executable (e.g. "compile-test"
# and "test")
SOURCES = src test
include targets.mk

# Include generated dependencies
-include $(OBJ_DIR)/*.d $(OBJ_DIR)/*/*.d
