CC = x86_64-w64-mingw32-g++
CFLAGS = -std=c++17 -O2 -Iinclude -Ilua/luajit/src -lenet
LDFLAGS = -lSDL3 -llua5.1 -lenet -lopengl32 -lgdi32 -lwinmm -limm32 -lole32 -loleaut32 -lversion -luuid -ladvapi32 -lshell32

SRCDIR = src
INCDIR = include
BUILDDIR = build
SERVERDIR = server

TARGET = $(BUILDDIR)/IrBox.exe
SERVER_TARGET = $(BUILDDIR)/IrBoxServer.exe

SOURCES = $(SRCDIR)/main.cpp
SERVER_SOURCES = $(SERVERDIR)/server.cpp

all: dirs $(TARGET) $(SERVER_TARGET) dlls

dirs:
	@mkdir -p $(BUILDDIR)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

$(SERVER_TARGET): $(SERVER_SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SERVER_SOURCES) $(LDFLAGS)

dlls:
	@echo Copying required DLLs...
	@cp -f lua/luajit/src/lua51.dll $(BUILDDIR)/ 2>/dev/null || echo "lua51.dll not found, please install LuaJIT"
	@cp -f libs/enet/.libs/libenet-1.3.18.dll $(BUILDDIR)/ 2>/dev/null || echo "libenet.dll not found, please build ENet"
	@echo DLL copy complete

studio:
	@echo Building IrBox Studio...
	$(CC) $(CFLAGS) -o $(BUILDDIR)/IrBoxStudio.exe studio/studio.cpp $(LDFLAGS) -limm32

clean:
	rm -rf $(BUILDDIR)/*.exe $(BUILDDIR)/*.dll

install:
	@echo Installing IrBox...
	@mkdir -p "C:/Program Files/IrBox"
	@cp $(BUILDDIR)/* "C:/Program Files/IrBox/"
	@echo Installation complete!

.PHONY: all dirs clean install dlls studio
