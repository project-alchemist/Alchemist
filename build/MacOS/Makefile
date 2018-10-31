include $(ELEMENTAL_PATH)/conf/ElVars

SRC_PATH = $(ALCHEMIST_PATH)/src/main
TARGET_PATH = $(ALCHEMIST_PATH)/target

# Put Elemental's CXXFLAGS in front so ours can override them
CXXFLAGS += $(EL_COMPILE_FLAGS) -fdiagnostics-color=always
CXXFLAGS += -Wno-unused -Wno-reorder -DASIO_STANDALONE -std=c++11 -fext-numeric-literals
CXXFLAGS += "-I$(ELEMENTAL_PATH)/include"
CXXFLAGS += "-I$(SPDLOG_PATH)/include"
CXXFLAGS += "-I$(EIGEN3_PATH)/include"
CXXFLAGS += "-I$(ARPACK_PATH)/include"

LDFLAGS += "-L$(EL_LIB)" "-Wl,-rpath,$(EL_LIB)" $(EL_LIBS)
LDFLAGS += -Wl,-rpath,$(ARPACK_PATH)/lib -larpack
LDFLAGS += -Wl,-rpath,$(SPDLOG_PATH)/lib -lspdlog
# LDFLAGS += -lboost_system-mt -lboost_thread-mt
LDFLAGS += -lmpi

OBJ_FILES = \
	$(TARGET_PATH)/main.o \
	$(TARGET_PATH)/Session.o \
	$(TARGET_PATH)/DriverSession.o \
	$(TARGET_PATH)/WorkerSession.o \
	$(TARGET_PATH)/LibraryManager.o \
	$(TARGET_PATH)/Server.o \
	$(TARGET_PATH)/Driver.o \
	$(TARGET_PATH)/Worker.o \
	$(TARGET_PATH)/GroupDriver.o \
	$(TARGET_PATH)/GroupWorker.o

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
