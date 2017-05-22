OPT = -g3 -O0
LIB_SOURCES = icy_mem.c icy_table.c log.c mem.c error.c array.c icy_vector.c
CC = gcc
TARGET = libicydb.so
LIB_OBJECTS =$(LIB_SOURCES:.c=.o)
LDFLAGS= -L. $(OPT) -Wextra -shared -fPIC
LIBS=
ALL= $(TARGET)
CFLAGS =  -I. -std=gnu11 -c $(OPT) -Wall -Wextra -Werror=implicit-function-declaration -Wformat=0 -D_GNU_SOURCE -fdiagnostics-color -Wextra  -Wwrite-strings -Werror -msse4.2 -mtune=corei7 -ffast-math -Werror=maybe-uninitialized -fPIC

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
.PHONY: test
test:
	make -f makefile.compiler
	make -f makefile.test

-include $(LIB_OBJECTS:.o=.o.depends)

