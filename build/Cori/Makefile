include $(ELEMENTAL_PATH)/conf/ElVars

SRC_PATH = $(ALCHEMIST_PATH)/src/main
TARGET_PATH = $(ALCHEMIST_PATH)/target

# Put Elemental's CXXFLAGS in front so ours can override them
CXXFLAGS += $(EL_COMPILE_FLAGS) -fdiagnostics-color=always
CXXFLAGS += -Wno-unused -Wno-reorder -DASIO_STANDALONE -std=c++11 -fext-numeric-literals -fopenmp
CXXFLAGS += "-I$(ELEMENTAL_PATH)/include"
CXXFLAGS += "-I$(SPDLOG_PATH)/include"
CXXFLAGS += "-I$(ARPACK_PATH)/include"
CXXFLAGS += "-I$(ASIO_PATH)/include"

LDLIBS = -lmpi -ldl

LDFLAGS += "-L$(EL_LIB)" "-Wl,-rpath,$(EL_LIB)" $(EL_LIBS)
LDFLAGS += "-Wl,-rpath,$(SPDLOG_PATH)/lib"

OBJ_FILES = \
	$(TARGET_PATH)/main.o \
	$(TARGET_PATH)/Session.o \
	$(TARGET_PATH)/DriverSession.o \
	$(TARGET_PATH)/WorkerSession.o \
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
	$(CXX) $(CXXFLAGS) -D_GLIBCXX_USE_CXX11_ABI=1 -o $@ $(OBJ_FILES) -rdynamic $(LDLIBS) $(LDFLAGS)

$(TARGET_PATH):
	mkdir -p $(TARGET_PATH)

.PHONY: clean
clean:
	rm -rf $(TARGET_PATH)





#include $(ELEMENTAL_PATH)/conf/ElVars
#
#SRC_PATH = $(ALCHEMIST_PATH)/src/main
#TARGET_PATH = $(ALCHEMIST_PATH)/target
#
## Put Elemental's CXXFLAGS in front so ours can override them
#CXXFLAGS += $(EL_COMPILE_FLAGS) -fdiagnostics-color=always
#CXXFLAGS += -Wno-unused -Wno-reorder -std=c++11 -fext-numeric-literals -fopenmp
#CXXFLAGS += "-I$(ELEMENTAL_PATH)/include" "-I$(SPDLOG_PATH)/include"
#CXXFLAGS += "-I$(BOOST_DIR)/include"									# some environments (e.g. Cori) require this to find boost
#
#LDFLAGS += "-L$(EL_LIB)" "-Wl,-rpath,$(EL_LIB)" $(EL_LIBS)
#LDFLAGS += -lboost_serialization -lboost_mpi -lboost_system -lboost_thread 
#LDFLAGS += -Wl,-rpath,$(SPDLOG_PATH)/lib
#LDFLAGS += -Wl,-rpath,$(BOOST_DIR)/lib "-L$(BOOST_DIR)/lib"			# some environments (e.g. Cori) require this to find boost
#LDFLAGS += -Wl,-rpath,/opt/intel/compilers_and_libraries_2016.3.210/linux/compiler/lib/intel64_lin/ -L/opt/intel/compilers_and_libraries_2016.3.210/linux/compiler/lib/intel64_lin/ -lirc
#
#OBJ_FILES = \
#	$(TARGET_PATH)/main.o \
#	$(TARGET_PATH)/Session.o \
#	$(TARGET_PATH)/DriverSession.o \
#	$(TARGET_PATH)/WorkerSession.o \
#	$(TARGET_PATH)/Server.o \
#	$(TARGET_PATH)/Driver.o \
#	$(TARGET_PATH)/Worker.o \
#	$(TARGET_PATH)/GroupDriver.o \
#	$(TARGET_PATH)/GroupWorker.o
#
#$(TARGET_PATH)/%.o: $(SRC_PATH)/%.cpp
#	$(CXX) -c $(CXXFLAGS) -D_GLIBCXX_USE_CXX11_ABI=1 $< -o $@
#
#.PHONY: default
#default: $(TARGET_PATH)/alchemist
#
#$(TARGET_PATH)/alchemist: $(TARGET_PATH) $(OBJ_FILES)
#	$(CXX) -dynamic $(CXXFLAGS) -D_GLIBCXX_USE_CXX11_ABI=1 -o $@ $(OBJ_FILES) $(LDFLAGS)
#
#$(TARGET_PATH):
#	mkdir -p $(TARGET_PATH)
#
#.PHONY: clean
#clean:
#	rm -rf $(TARGET_PATH)
