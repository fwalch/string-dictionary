define generate_targets
ifneq ($($(1)_GENERATED),1)
$(eval $(1)_GENERATED=1)
$(eval include $(1)/$(1).mk)

# Source variables
$(eval $(1)_SOURCES = $(addprefix $(1)/, $(call $(1)_sources)))
$(eval $(1)_EXECUTABLES = $(call $(1)_executables))
$(eval $(1)_ALL_SOURCES = $($(1)_SOURCES) $(addprefix $(1)/, $(addsuffix .cpp, $($(1)_EXECUTABLES))))
$(eval $(1)_DEPENDENCIES = $(call $(1)_dependencies))
EXECUTABLES += $($(1)_EXECUTABLES)

$(foreach dep,$($(1)_DEPENDENCIES),$(eval $(call generate_targets,$(dep))))

# Object variables
$(eval $(1)_ALL_OBJECTS = $(addprefix $(OBJ_DIR)/,$($(1)_ALL_SOURCES:%.cpp=%.o)))
$(eval $(1)_SRC_OBJECTS = $(addprefix $(OBJ_DIR)/,$($(1)_SOURCES:%.cpp=%.o)))
$(eval $(1)_EXE_OBJECTS = $(addprefix $(EXE_DIR)/,$($(1)_EXECUTABLES:%.cpp=%.o)))
$(eval $(1)_DEP_OBJECTS = $(foreach dep, $($(1)_DEPENDENCIES),$($(dep)_SRC_OBJECTS)))

# Flags variables
$(eval $(1)_CXXFLAGS = $(call $(1)_cxxflags))
$(eval $(1)_LDFLAGS = $(call $(1)_ldflags))
$(eval $(1)_DEP_LDFLAGS = $(foreach dep, $($(1)_DEPENDENCIES),$($(dep)_LDFLAGS)))
$(eval $(1)_DEP_INCLUDES = $(addprefix -isystem ,$($(1)_DEPENDENCIES) $($(1)_DEPENDENCIES:%=%/include)))
$(eval $(1)_INCLUDES = -I $(1)/include)

# Targets
.SECONDEXPANSION:
$($(1)_ALL_OBJECTS): $(OBJ_DIR)/%.o: %.cpp
	$(CHECKDIR)
	$(BUILDOBJ) $($(1)_INCLUDES) $($(1)_CXXFLAGS) $($(1)_DEP_INCLUDES)

$($(1)_EXE_OBJECTS): $(EXE_DIR)/%: $(OBJ_DIR)/$(1)/%.o $($(1)_DEP_OBJECTS) $($(1)_ALL_OBJECTS)
	$(CHECKDIR)
	$(BUILDEXE) $($(1)_LDFLAGS) $($(1)_DEP_LDFLAGS)
endif
endef

EXECUTABLES=
$(foreach source, $(SOURCES), $(eval $(call generate_targets,$(source))))

# General targets
$(EXECUTABLES:%=compile-%): compile-%: $(EXE_DIR)/%
$(EXECUTABLES): %: $(EXE_DIR)/%
	./$<

