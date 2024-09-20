CC = gcc

CFLAGS = -Wall -g -Iinclude

# Target executables
CLIENT_TARGET = play/client
SERVER_TARGET = play/server

# Source files
SRCS = src/Game.c src/Client.c src/Server.c

# Header files
HEADERS = include/Resources.h

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: create_play_dir $(CLIENT_TARGET) $(SERVER_TARGET)
	rm -f $(OBJS)

# Create Play directory if it doesn't exist
create_play_dir:
	mkdir -p play

# Link object files to create the client executable
$(CLIENT_TARGET): src/Game.o src/Client.o
	$(CC) $(CFLAGS) -o $(CLIENT_TARGET) src/Game.o src/Client.o

# Link object files to create the server executable
$(SERVER_TARGET): src/Game.o src/Server.o
	$(CC) $(CFLAGS) -o $(SERVER_TARGET) src/Game.o src/Server.o

src/%.o: src/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf play

.PHONY: all clean
