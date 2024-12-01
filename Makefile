# Compiler
CC = gcc

# Directories
# INCLUDE_DIR = ./src/include
# LIB_DIR = ./src/lib

LIB_DIR = ./src/lib
INCLUDE_DIR = ./src/include

# Compiler Flags
CFLAGS = -I$(INCLUDE_DIR)
LDFLAGS = -L$(LIB_DIR) -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_ttf
# LDFLAGS = -L$(LIB_DIR) -lSDL2 -lSDL2main
# LDFLAGS = -L$(LIB_DIR) -lSDL2 -lSDL2main




# Source Files
SRC = main.c

# Output Executable
OUT = game

# Build Rule
all: $(OUT)

$(OUT): $(SRC)
	$(CC) $(SRC) -o $(OUT) $(CFLAGS) $(LDFLAGS)

# Clean Rule
clean:
	rm -f $(OUT)
