OPT = -g3 -O4
LIB_SOURCES =  iron/mem.c iron/process.c iron/array.c iron/math.c iron/time.c  iron/log.c iron/fileio.c iron/linmath.c iron/test.c iron/error.c iron/image.c iron/gl.c iron/coroutines2.c main.c persist.c shader_utils.c hsv.c gui_test.c persist_oop.c stb_truetype.c gui.c console.c stb_image.c animation.c command.c game_board.c sortable.c simple_graphics.c simple_graphics_editor.c abstract_sortable.c index_table.c #gui_demo.c #hydra.c #elf_reader.c type_system.c #game.c main2.c
CC = gcc
TARGET = run.exe
LIB_OBJECTS =$(LIB_SOURCES:.c=.o)
LDFLAGS= -L. $(OPT) -Wextra -fopenmp -rdynamic #-Wl,-stack_size,0x100000000  #-ftlo #setrlimit on linux 
LIBS= -ldl -lm -lGL -lpthread -lglfw -lGLEW -lpng -lmill -lecl #-lmcheck
ALL= $(TARGET)
CFLAGS =  -I. -std=gnu11 -gdwarf-2 -c $(OPT) -Wall -Wextra -Werror=implicit-function-declaration -Wformat=0 -D_GNU_SOURCE -fdiagnostics-color -Wextra  -Wwrite-strings -Werror -msse4.2 -mtune=corei7 -fopenmp -ffast-math -Werror=maybe-uninitialized # -Wsuggest-attribute=const #-DDEBUG  

$(TARGET): $(LIB_OBJECTS)
	$(CC) $(LDFLAGS) $(LIB_OBJECTS) $(LIBS) -o $@

all: $(ALL)

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -MMD -MF $@.depends 
depend: h-depend
clean:
	rm -f $(LIB_OBJECTS) $(ALL) *.o.depends

-include $(LIB_OBJECTS:.o=.o.depends)

