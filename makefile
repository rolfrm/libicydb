OPT = -g3 -O0
LIB_SOURCES = icy_mem.c icy_table.c log.c mem.c error.c array.c icy_vector.c
CC = gcc
TARGET = libicydb.so
LIB_OBJECTS =$(LIB_SOURCES:.c=.o)
LDFLAGS= -L. $(OPT) -Wextra -shared -fPIC #-Wl,-stack_size,0x100000000  #-ftlo #setrlimit on linux 
LIBS= #-ldl -lm -lGL -lpthread -lglfw -lGLEW -lpng #-lmcheck
ALL= $(TARGET)
CFLAGS =  -I. -std=gnu11 -gdwarf-2 -c $(OPT) -Wall -Wextra -Werror=implicit-function-declaration -Wformat=0 -D_GNU_SOURCE -fdiagnostics-color -Wextra  -Wwrite-strings -Werror -msse4.2 -mtune=corei7 -fopenmp -ffast-math -Werror=maybe-uninitialized -fPIC # -Wsuggest-attribute=const #-DDEBUG  

$(TARGET): $(LIB_OBJECTS)
	$(CC) $(LDFLAGS) $(LIB_OBJECTS) $(LIBS) -o $@

all: $(ALL)

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -MMD -MF $@.depends 
depend: h-depend
clean:
	rm -f $(LIB_OBJECTS) $(ALL) *.o.depends
install: $(TARGET)
	cp include/* /usr/include
	cp $(TARGET) /usr/lib/


-include $(LIB_OBJECTS:.o=.o.depends)

