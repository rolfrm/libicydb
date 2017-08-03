OPT = -g3 -O0
LIB_SOURCES1 = icy_mem.c icy_table.c log.c mem.c error.c array.c icy_vector.c icy_vector_abs.c
LIB_SOURCES = $(addprefix src/, $(LIB_SOURCES1))
CC = gcc
TARGET = libicydb.so
LIB_OBJECTS =$(LIB_SOURCES:.c=.o)
LDFLAGS= -L. $(OPT) -Wextra -shared -fPIC
LIBS=
ALL= $(TARGET)
CFLAGS = -Isrc/ -Iinclude/ -std=gnu11 -c $(OPT) -Wall -Wextra -Werror=implicit-function-declaration -Wformat=0 -D_GNU_SOURCE -fdiagnostics-color -Wextra  -Wwrite-strings -Werror -msse4.2 -mtune=corei7 -ffast-math -Werror=maybe-uninitialized -fPIC -DUSE_VALGRIND



$(TARGET): $(LIB_OBJECTS)
	$(CC) $(LDFLAGS) $(LIB_OBJECTS) $(LIBS) -o $@

all: $(ALL)

.c.o: $(HEADERS)
	$(CC) $(CFLAGS) $< -o $@ -MMD -MF $@.depends 
depend: h-depend
clean:
	rm -f $(LIB_OBJECTS) $(ALL) src/*.o.depends
install: $(TARGET)
	make -f makefile.compiler
	cp -v include/* /usr/include/	
	cp -v $(TARGET) /usr/lib/
	cp -v icy_table_template.c icy_table_template.h /usr/bin/
	cp -v icy-table /usr/bin/
uninstall:
	rm -v /usr/include/icy_* || true
	rm -v /usr/lib/$(TARGET) || true
	rm -v /usr/bin/icy-table || true
	rm -v /usr/bin/icy_* || true
	echo "done.."
.PHONY: test
test: $(TARGET)
	make -f makefile.compiler
	make -f makefile.test test

-include $(LIB_OBJECTS:.o=.o.depends)

