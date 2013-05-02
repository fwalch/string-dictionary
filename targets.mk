define generate_targets
$(eval include $(1)/$(1).mk)

# Source variables
$(eval $(1)_SOURCES = $(addprefix $(1)/, $(call $(1)_sources)))
$(eval $(1)_EXECUTABLES = $(call $(1)_executables))
$(eval $(1)_ALL_SOURCES = $($(1)_SOURCES) $(addprefix $(1)/, $(addsuffix .cpp, $($(1)_EXECUTABLES))))
$(eval $(1)_DEPENDENCIES = $(call $(1)_dependencies))
EXECUTABLES += $($(1)_EXECUTABLES)

# Object variables
$(eval $(1)_ALL_OBJECTS = $(addprefix $(OBJ_DIR)/,$($(1)_ALL_SOURCES:%.cpp=%.o)))
$(eval $(1)_SRC_OBJECTS = $(addprefix $(OBJ_DIR)/,$($(1)_SOURCES:%.cpp=%.o)))
$(eval $(1)_EXE_OBJECTS = $(addprefix $(EXE_DIR)/,$($(1)_EXECUTABLES:%.cpp=%.o)))
$(eval $(1)_DEP_OBJECTS = $(foreach dep, $($(1)_DEPENDENCIES),$($(dep)_SRC_OBJECTS)))

# Flags variables
$(eval $(1)_CXXFLAGS = $(call $(1)_cxxflags))
$(eval $(1)_LDFLAGS = $(call $(1)_ldflags))
$(eval $(1)_DEP_LDFLAGS = $(foreach dep, $($(1)_DEPENDENCIES),$($(dep)_LDFLAGS)))

# Targets
.SECONDEXPANSION:
$($(1)_ALL_OBJECTS): $(OBJ_DIR)/%.o: %.cpp
	$(CHECKDIR)
	$(BUILDOBJ) $($(1)_CXXFLAGS)

$($(1)_EXE_OBJECTS): $(EXE_DIR)/%: $(OBJ_DIR)/$(1)/%.o $($(1)_DEP_OBJECTS) $($(1)_ALL_OBJECTS)
	$(CHECKDIR)
	$(BUILDEXE) $($(1)_LDFLAGS) $($(1)_DEP_LDFLAGS)
endef

EXECUTABLES=
$(foreach source, $(SOURCES), $(eval $(call generate_targets,$(source))))

# General targets
$(EXECUTABLES:%=compile-%): compile-%: $(EXE_DIR)/%
$(EXECUTABLES): %: $(EXE_DIR)/%
	./$<

