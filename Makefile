CXX=g++
CXX_WIN=x86_64-w64-mingw32-g++-win32
CPPFLAGS=-g -flto -O0 -std=c++20 -I$(SRC) -D DEBUG
LDFLAGS=-g -flto -O0
RM=rm -f
SRC=src
BUILD=build
BUILD_LIN=$(BUILD)/linux
BUILD_WIN=$(BUILD)/win
DIST=bin

APP=wh-asm
APP_DIR=$(DIST)/$(APP)

SRCS=$(shell find $(SRC) -name *.cpp)
OBJS_LIN=$(patsubst $(SRC)/%.cpp, $(BUILD_LIN)/%.o, $(SRCS))
OBJS_WIN=$(patsubst $(SRC)/%.cpp, $(BUILD_WIN)/%.o, $(SRCS))

all: win linux

$(OBJS_LIN): $(SRCS)
	mkdir -p $(@D)
	@echo > $@
	$(CXX) $(CPPFLAGS) -o $@ -c $(patsubst $(BUILD_LIN)/%.o, $(SRC)/%.cpp, $@)

$(OBJS_WIN): $(SRCS)
	mkdir -p $(@D)
	@echo > $@
	$(CXX_WIN) $(CPPFLAGS) -o $@ -c $(patsubst $(BUILD_WIN)/%.o, $(SRC)/%.cpp, $@)

linux: $(OBJS_LIN)
	mkdir -p $(DIST)
	$(CXX) $(LDFLAGS) -o $(APP_DIR) $(OBJS_LIN)

win: $(OBJS_WIN)
	mkdir -p $(DIST)
	$(CXX_WIN) $(LDFLAGS) -o $(APP_DIR).exe $(OBJS_WIN)

clean:
	$(RM) $(OBJS_LIN) $(OBJS_WIN)

distclean: clean
	$(RM) $(APP_DIR) $(APP_DIR).exe

TASM_DIR:=../../assembly/dos-tasm
DOS=dosbox
DOS_FLAGS=-c 'mount C "$(TASM_DIR)"' -c 'mount D "$(shell pwd)"' -c D: -c 'set PATH=C:\' -c cls
ASM=nasm

dos:
	$(DOS) $(DOS_FLAGS)
	
my-asm:
	$(APP_DIR) source.asm

profile:
	valgrind -q --tool=callgrind --callgrind-out-file=callgrind.out ./bin/wh-asm
	gprof2dot --format=callgrind --output=callgrind.dot callgrind.out
	dot -Tpng callgrind.dot -o trace.png
	sudo xdg-open trace.png

asm:
	$(ASM) -o test-nasm.com source.asm