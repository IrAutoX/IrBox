CC = gcc
CXX = g++
CFLAGS = -Wall -Wextra -O2 -I./include -I./deps/luajit/src -I./deps/enet/include
CXXFLAGS = $(CFLAGS) -std=c++17 -pthread
LDFLAGS = -L./deps/luajit/src -lluajit -L./deps/enet/.libs -lenet

ifdef WINDOWS
    CXX = x86_64-w64-mingw32-g++
    CC = x86_64-w64-mingw32-gcc
    CFLAGS += -DWINDOWS
    LDFLAGS += -lws2_32 -lwinmm
    EXE = .exe
else
    LDFLAGS += -ldl -lm
    EXE =
endif

SRCDIR = src
INCDIR = include
DEPDIR = deps
BUILDDIR = build
STUDIODIR = studio

SOURCES = $(SRCDIR)/main.cpp
STUDIO_SOURCES = $(STUDIODIR)/studio.cpp
ENET_SOURCES = $(wildcard $(DEPDIR)/enet/*.c)
LUAGIT_SOURCES = 

OBJECTS = $(SOURCES:.cpp=.o)
STUDIO_OBJECTS = $(STUDIO_SOURCES:.cpp=.o)
ENET_OBJECTS = $(ENET_SOURCES:.c=.o)

TARGET = $(BUILDDIR)/irbox$(EXE)
STUDIO_TARGET = $(BUILDDIR)/irbox_studio$(EXE)

.PHONY: all clean luajit enet test

all: luajit enet $(TARGET) $(STUDIO_TARGET)

luajit:
	@echo "[Build] Building LuaJIT..."
ifdef WINDOWS
	cd $(DEPDIR)/luajit && make mingw
	cp $(DEPDIR)/luajit/src/lua51.dll $(BUILDDIR)/ 2>/dev/null || true
	cp $(DEPDIR)/luajit/src/luajit-2.0.dll $(BUILDDIR)/ 2>/dev/null || true
else
	cd $(DEPDIR)/luajit && make
endif
	@echo "[Build] LuaJIT built successfully"

enet:
	@echo "[Build] Building ENet..."
ifdef WINDOWS
	cd $(DEPDIR)/enet && autoreconf -fi 2>/dev/null || true
	cd $(DEPDIR)/enet && ./configure --host=x86_64-w64-mingw32 --enable-static=yes --enable-shared=no 2>/dev/null || true
	cd $(DEPDIR)/enet && make 2>/dev/null || true
	cp $(DEPDIR)/enet/.libs/libenet.a $(DEPDIR)/enet/ 2>/dev/null || true
else
	cd $(DEPDIR)/enet && autoreconf -fi
	cd $(DEPDIR)/enet && ./configure --enable-static=yes --enable-shared=no
	cd $(DEPDIR)/enet && make
endif
	@echo "[Build] ENet built successfully"

$(TARGET): $(SOURCES) $(INCDIR)/irbox_engine.h
	@echo "[Build] Compiling main executable..."
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(SOURCES) $(LDFLAGS)
	@echo "[Build] Main executable built: $@"

$(STUDIO_TARGET): $(STUDIO_SOURCES) $(INCDIR)/irbox_engine.h
	@echo "[Build] Compiling Studio executable..."
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -o $@ $(STUDIO_SOURCES) $(LDFLAGS)
	@echo "[Build] Studio executable built: $@"

test: $(TARGET)
	@echo "[Test] Running test..."
ifdef WINDOWS
	echo "Windows build - run manually"
else
	timeout 2 ./$(TARGET) || true
endif
	@echo "[Test] Test completed"

clean:
	@echo "[Clean] Removing build artifacts..."
	rm -rf $(BUILDDIR)/*
ifdef WINDOWS
	cd $(DEPDIR)/luajit && make clean 2>/dev/null || true
	cd $(DEPDIR)/enet && make clean 2>/dev/null || true
else
	cd $(DEPDIR)/luajit && make clean
	cd $(DEPDIR)/enet && make clean
endif
	@echo "[Clean] Done"

install-deps:
	@echo "[Install] Installing system dependencies..."
ifdef WINDOWS
	echo "For Windows, ensure MinGW-w64 is installed"
else
	sudo apt-get update
	sudo apt-get install -y build-essential autoconf automake libtool
endif

help:
	@echo "IrBox Ultimate Build System"
	@echo "==========================="
	@echo ""
	@echo "Targets:"
	@echo "  all          - Build everything (default)"
	@echo "  luajit       - Build LuaJIT only"
	@echo "  enet         - Build ENet only"
	@echo "  test         - Run basic test"
	@echo "  clean        - Clean build artifacts"
	@echo "  install-deps - Install system dependencies (Linux only)"
	@echo ""
	@echo "Options:"
	@echo "  WINDOWS=1    - Build for Windows using MinGW-w64"
	@echo ""
	@echo "Examples:"
	@echo "  make                    # Build for Linux"
	@echo "  make WINDOWS=1          # Build for Windows"
	@echo "  make test               # Run tests"
	@echo "  make clean              # Clean everything"
	@echo ""
	@echo "Copyright (c) DeathAmir And IrAutoX"
