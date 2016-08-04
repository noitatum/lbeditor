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
# Source directory
SRC_DIR = src/
# Object directory
OBJ_DIR = obj/
# Object files
OBJS = main.o sprite.o stage.o render.o hud.o action.o
# Object full path
OBJS_FULL = $(addprefix $(OBJ_DIR), $(OBJS))
# SL2 MINGW directory
SDL2_MINGW_DIR = SDL2/

all: $(DIRECTORIES) $(EXECUTABLE) 

windows: CC=i686-w64-mingw32-gcc
windows: CFLAGS+=-I$(SDL2_MINGW_DIR)i686-w64-mingw32/include/ -Dmain=SDL_main
windows: LFLAGS=-L$(SDL2_MINGW_DIR)i686-w64-mingw32/lib/ -lmingw32 -lSDL2main -lSDL2 -mwindows
windows: EXECUTABLE=bin/moon.exe
windows: all
	cp $(SDL2_MINGW_DIR)lib/x86/SDL2.dll bin/

clean:
	rm -rf $(DIRECTORIES)

debug: OPT_FLAGS=-ggdb
debug: all

$(DIRECTORIES):
	mkdir -p $@

$(EXECUTABLE): $(OBJS_FULL)
	$(CC) $^ $(LFLAGS) $(OPT_FLAGS) -o $(EXECUTABLE)

$(OBJ_DIR)%.o: $(SRC_DIR)%.c
	$(CC) $< $(CFLAGS) $(OPT_FLAGS) -c -o $@
