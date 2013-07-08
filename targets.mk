define generate_targets
ifneq ($($(1)_GENERATED),1)
$(eval $(1)_GENERATED=1)
$(eval include $(1)/$(1).mk)

# Source variables
$(eval $(1)_SOURCES = $(addprefix $(1)/, $(call $(1)_sources)))
$(eval $(1)_CSOURCES = $(addprefix $(1)/, $(call $(1)_csources)))
$(eval $(1)_EXECUTABLES = $(call $(1)_executables))
$(eval $(1)_ALL_SOURCES = $($(1)_SOURCES) $(addprefix $(1)/, $(addsuffix .cpp, $($(1)_EXECUTABLES))))
$(eval $(1)_ALL_CSOURCES = $($(1)_CSOURCES))
$(eval $(1)_DEPENDENCIES = $(call $(1)_dependencies))
$(eval $(1)_LIBRARIES = $(call $(1)_libraries))
EXECUTABLES += $($(1)_EXECUTABLES)

# Generate dependencies
$(foreach dep,$($(1)_DEPENDENCIES),$(eval $(call generate_targets,$(dep))))
$(foreach dep,$($(1)_LIBRARIES),$(eval $(call generate_targets,$(dep))))

# Object variables
$(eval $(1)_ALL_OBJECTS = $(addprefix $(OBJ_DIR)/,$($(1)_ALL_SOURCES:%.cpp=%.o)))
$(eval $(1)_ALL_COBJECTS = $(addprefix $(OBJ_DIR)/,$($(1)_ALL_CSOURCES:%.c=%.o)))
$(eval $(1)_SRC_OBJECTS = $(addprefix $(OBJ_DIR)/,$($(1)_SOURCES:%.cpp=%.o)))
$(eval $(1)_SRC_COBJECTS = $(addprefix $(OBJ_DIR)/,$($(1)_CSOURCES:%.c=%.o)))
$(eval $(1)_EXE_OBJECTS = $(addprefix $(EXE_DIR)/,$($(1)_EXECUTABLES:%.cpp=%.o)))
$(eval $(1)_DEP_OBJECTS = $(foreach dep, $($(1)_DEPENDENCIES),$($(dep)_SRC_OBJECTS) $($(dep)_SRC_COBJECTS) $($(dep)_LIB_OBJECTS)))
$(eval $(1)_LIB_OBJECTS = $(foreach lib, $($(1)_LIBRARIES),$($(lib)_SRC_OBJECTS) $($(lib)_SRC_COBJECTS)))

# Flags variables
$(eval $(1)_CXXFLAGS = $(call $(1)_cxxflags))
$(eval $(1)_CFLAGS = $(call $(1)_cflags))
$(eval $(1)_LDFLAGS = $(call $(1)_ldflags))
$(eval $(1)_DEP_LDFLAGS = $(foreach dep, $($(1)_DEPENDENCIES),$($(dep)_LDFLAGS)))
$(eval $(1)_DEP_INCLUDES = $(addprefix -I ,$($(1)_DEPENDENCIES) $($(1)_DEPENDENCIES:%=%/include)) $(foreach dep, $($(1)_DEPENDENCIES),$($(dep)_LIB_INCLUDES)))
$(eval $(1)_LIB_LDFLAGS = $(foreach lib, $($(1)_LIBRARIES),$($(lib)_LDFLAGS)))
$(eval $(1)_LIB_INCLUDES = $(addprefix -isystem ,$($(1)_LIBRARIES) $($(1)_LIBRARIES:%=%/include)))
$(eval $(1)_INCLUDES = -I $(1)/include)

# Targets
.SECONDEXPANSION:
$($(1)_ALL_OBJECTS): $(OBJ_DIR)/%.o: %.cpp
	$(eval $(CXXFLAGS += $($(BUILD)FLAGS)))
	$(CHECKDIR)
	$(BUILDCXX) $($(1)_INCLUDES) $($(1)_DEP_INCLUDES) $($(1)_LIB_INCLUDES) $($(1)_CXXFLAGS)

$($(1)_ALL_COBJECTS): $(OBJ_DIR)/%.o: %.c
	$(eval $(CCFLAGS += $($(BUILD)FLAGS)))
	$(CHECKDIR)
	$(BUILDCC) $($(1)_INCLUDES) $($(1)_DEP_INCLUDES) $($(1)_LIB_INCLUDES) $($(1)_CFLAGS)

$($(1)_EXE_OBJECTS): $(EXE_DIR)/%: $(OBJ_DIR)/$(1)/%.o $($(1)_DEP_OBJECTS) $($(1)_LIB_OBJECTS) $($(1)_SRC_OBJECTS) $$<
	$(eval $(LDFLAGS += $($(BUILD)FLAGS)))
	$(CHECKDIR)
	$(BUILDEXE) $($(1)_DEP_LDFLAGS) $($(1)_LIB_LDFLAGS) $($(1)_LDFLAGS)
endif
endef

EXECUTABLES=
$(foreach source, $(SOURCES), $(eval $(call generate_targets,$(source))))

# General targets
$(EXECUTABLES:%=compile-%): compile-%: prepare-compile $(EXE_DIR)/%
$(EXECUTABLES:%=debug-%): debug-%: set-debug compile-%
prepare-compile:
	$(eval CXXFLAGS += $($(BUILD)FLAGS))
	$(eval CFLAGS += $($(BUILD)FLAGS))
	$(eval LDFLAGS += $($(BUILD)FLAGS))
compile-executables: $(EXECUTABLES:%=compile-%)
debug-executables: $(EXECUTABLES:%=debug-%)
