# Object files
OBJS = main.o sprite.o stage.o render.o hud.o action.o
# Object full path
OBJS_FULL = $(addprefix $(OBJ_DIR), $(OBJS))
# Compiler
CC = gcc
# Compiler flags
CFLAGS = -std=c11 -Wall -Wextra -Wno-unused-result -Werror -Isrc/include/ 
# Optimization flags
OPT_FLAGS = -O2 -flto
# Linker flags
LFLAGS = -lSDL2
# Executable
EXECUTABLE = bin/moon 
# Build Directories
DIRECTORIES = bin/ obj/
# Source dir
SRC_DIR = src/
# Object dir
OBJ_DIR = obj/

all: $(DIRECTORIES) $(EXECUTABLE) 

clean:
	rm -rf $(DIRECTORIES)

debug: OPT_FLAGS=-ggdb
debug: all

$(DIRECTORIES):
	mkdir -p $@

$(EXECUTABLE): $(OBJS_FULL)
	$(CC) $^ $(LFLAGS) $(OPT_FLAGS) -o $@

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $< $(CFLAGS) $(OPT_FLAGS) -c -o $@
