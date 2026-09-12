CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -O2 -I./include
LDFLAGS = -llua5.1 -lpthread

ifdef WINDOWS
    CFLAGS += -DWIN32_LEAN_AND_MEAN
    LDFLAGS += -lws2_32 -lopengl32 -lgdi32
endif

CLIENT_SRC = src/main.cpp
SERVER_SRC = server/server.cpp
CLIENT_BIN = build/IrBox.exe
SERVER_BIN = build/IrBoxServer.exe

all: client server

client: $(CLIENT_SRC) include/irbox_engine.h
	@echo Building IrBox Client...
	$(CC) $(CFLAGS) -o $(CLIENT_BIN) $(CLIENT_SRC) $(LDFLAGS)
	@echo Client built successfully!

server: $(SERVER_SRC) include/irbox_engine.h
	@echo Building IrBox Server...
	$(CC) $(CFLAGS) -o $(SERVER_BIN) $(SERVER_SRC) $(LDFLAGS)
	@echo Server built successfully!

clean:
	@echo Cleaning build directory...
	rm -f $(CLIENT_BIN) $(SERVER_BIN)
	@echo Clean complete!

run-client: client
	@echo Running IrBox Client...
	./$(CLIENT_BIN)

run-server: server
	@echo Running IrBox Server...
	./$(SERVER_BIN) 8080

package: all
	@echo Packaging IrBox...
	zip -r IrBox.zip build/ assets/ scripts/ logo.svg README.md
	@echo Package created: IrBox.zip

.PHONY: all clean run-client run-server package client server