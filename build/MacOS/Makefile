include $(ELEMENTAL_PATH)/conf/ElVars

SRC_PATH = $(ALCHEMIST_PATH)/src/main
TARGET_PATH = $(ALCHEMIST_PATH)/target
EXE_NAME = alchemist

# Put Elemental's CXXFLAGS in front so ours can override them
CXXFLAGS += $(EL_COMPILE_FLAGS) -fdiagnostics-color=always
CXXFLAGS += -Wno-unused -Wno-reorder -DASIO_STANDALONE -std=c++11 -fext-numeric-literals
CXXFLAGS += "-I$(ELEMENTAL_PATH)/include"
CXXFLAGS += -D_GLIBCXX_USE_CXX11_ABI=1

LDLIBS += -lmpi

LDFLAGS += "-L$(EL_LIB)" "-Wl,-rpath,$(EL_LIB)" $(EL_LIBS)

OBJ_FILES = main.o Session.o DriverSession.o WorkerSession.o Server.o Driver.o Worker.o GroupDriver.o GroupWorker.o
	
# Debug build settings
DBG_PATH = $(TARGET_PATH)/debug
DBG_EXE = $(DBG_PATH)/$(EXE_NAME)
DBG_OBJS = $(addprefix $(DBG_PATH)/, $(OBJ_FILES))
DBG_CFLAGS = -g -O0 -DDEBUG

# Release build settings
REL_PATH = $(TARGET_PATH)/release
REL_EXE = $(REL_PATH)/$(EXE_NAME)
REL_OBJS = $(addprefix $(REL_PATH)/, $(OBJ_FILES))
REL_CFLAGS = -O3 -DNDEBUG

.PHONY: all clean debug release

# Default build
all: prep debug

# Debug build
debug: $(DBG_EXE)

$(DBG_EXE): $(DBG_OBJS)
	$(CXX) -dynamic $(CXXFLAGS) $(DBG_CFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)
	
$(DBG_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(DBG_CFLAGS) -o $@ $<

# Release build
release: $(REL_EXE)

$(REL_EXE): $(REL_OBJS)
	$(CXX) -dynamic $(CXXFLAGS) $(REL_CFLAGS) -o $@ $^ $(LDLIBS) $(LDFLAGS)
	
$(REL_PATH)/%.o: $(SRC_PATH)/%.cpp
	$(CXX) -c $(CXXFLAGS) $(REL_CFLAGS) -o $@ $<

remake: clean all

prep:
	@mkdir -p $(TARGET_PATH) $(DBG_PATH) $(REL_PATH)

clean:
	rm -rf $(TARGET_PATH)
	
	
	
	



