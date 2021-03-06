# Redefine CXX only if not passed from environment variable
ifeq ($(origin CXX), default)
	ifneq ($(shell which clang++ 2>/dev/null),)
		CXX = clang++
	else
		CXX = g++
	endif
endif
# Redefine CC only if not passed from environment variable
ifeq ($(origin CC), default)
	ifneq ($(shell which clang 2>/dev/null),)
		CC = clang
	else
		CC = gcc
	endif
endif

# Define clang flags
ifeq ($(CXX),clang++)
	CXXFLAGS = -g -Weverything -Werror -Wno-c++98-compat -Wno-c++98-compat-pedantic -Wno-padded -Wno-documentation -Wno-weak-template-vtables -Wno-shadow -Wno-switch-enum -Wno-vla --std=c++11
endif
ifeq ($(shell clang --version | grep ^clang | sed -e 's/(.*)//g' -e 's/[^0-9.]*//g'),3.3)
	CXXFLAGS += -Wno-vla-extension
endif
ifeq ($(CC),clang)
	CFLAGS = -g -Weverything -Werror --std=c99
endif

# Define gcc flags
ifeq ($(CXX),g++)
	CXXFLAGS = -g -Wall -Wextra -Werror -Wno-type-limits -Wno-maybe-uninitialized -Wno-strict-aliasing -Wno-attributes --std=c++11
endif
ifeq ($(CC),gcc)
	CFLAGS = -g -Wall -Wextra -Werror --std=c99
endif

# Build type flags
DBGFLAGS := -O0 -DDEBUG
RELFLAGS := -O3

# Configuration variables
OBJ_DIR := obj
EXE_DIR := bin

CHECKDIR  = mkdir -p $$(dir $$@)
BUILDCXX  = $(CXX) -c $$(CXXFLAGS) -MD -MF $$(@:%.o=%.d) $$< -o $$@
BUILDCC   = $(CC) -c $$(CFLAGS) -MD -MF $$(@:%.o=%.d) $$< -o $$@
BUILDEXE  = $(CXX) $$(LDFLAGS) $$^ -o $$@

# Default build type
BUILD = DBG

# Main targets
all: compile-executables
debug: debug-executables
release: release-executables
clean:
	find $(OBJ_DIR) -type f -and -name '*.o' -delete 2> /dev/null || true
	find $(EXE_DIR) -type f -and -not -name '*.sh' -delete 2> /dev/null || true
purge: clean
	rm -rf $(OBJ_DIR)

# Helper targets
set-debug:
	$(eval BUILD = DBG)
set-release:
	$(eval BUILD = REL)

# Generate targets for source trees
SOURCES = src test
include targets.mk

# Include generated dependencies
-include $(OBJ_DIR)/*.d $(OBJ_DIR)/*/*.d
