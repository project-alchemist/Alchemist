include $(ELEMENTAL_PATH)/conf/ElVars

SRC_PATH = $(ALCHEMIST_PATH)/src/main
TARGET_PATH = $(ALCHEMIST_PATH)/target

# Put Elemental's CXXFLAGS in front so ours can override them
CXXFLAGS += $(EL_COMPILE_FLAGS) -fdiagnostics-color=always
CXXFLAGS += -Wno-unused -Wno-reorder -std=c++11 -fext-numeric-literals -fopenmp
CXXFLAGS += "-I$(ELEMENTAL_PATH)/include" "-I$(SPDLOG_PATH)/include"

LDFLAGS += "-L$(EL_LIB)" "-Wl,-rpath,$(EL_LIB)" $(EL_LIBS)
LDFLAGS += -lboost_serialization-mt -lboost_mpi-mt -lboost_system-mt -lboost_thread-mt
LDFLAGS += -Wl,-rpath,$(SPDLOG_PATH)/lib
LDFLAGS += -lmpi

OBJ_FILES = \
	$(TARGET_PATH)/main.o \
	$(TARGET_PATH)/Session.o \
	$(TARGET_PATH)/LibraryManager.o \
	$(TARGET_PATH)/Server.o \
	$(TARGET_PATH)/Driver.o \
	$(TARGET_PATH)/Worker.o

$(TARGET_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) -c $(CXXFLAGS) -D_GLIBCXX_USE_CXX11_ABI=1 $< -o $@

.PHONY: default
default: $(TARGET_PATH)/alchemist

$(TARGET_PATH)/alchemist: $(TARGET_PATH) $(OBJ_FILES)
	$(CXX) -dynamic $(CXXFLAGS) -D_GLIBCXX_USE_CXX11_ABI=1 -o $@ $(OBJ_FILES) $(LDFLAGS)

$(TARGET_PATH):
	mkdir -p $(TARGET_PATH)

.PHONY: clean
clean:
	rm -rf $(TARGET_PATH)
