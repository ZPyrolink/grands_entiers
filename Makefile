CC=clang
COMMON_FLAGS=-fsanitize=address -fno-omit-frame-pointer
CFLAGS=-g -Wall -Werror -include my_memory.h -DTRACK_ALLOCS $(COMMON_FLAGS)
# Additionnal flags to get a more verbose information (can be safely removed if needed)
CFLAGS+=-DVERBOSE -DDEBUG
LDFLAGS=$(COMMON_FLAGS)
PROGRAMS=test

.PHONY: clean

all:
	@echo "Tapez 'make program' où programme est parmi : $(PROGRAMS)"

test: test.o grand_entier.o my_memory.o

grand_entier.o: grand_entier.h
test.o: grand_entier.h

clean:
	rm *.o $(PROGRAMS)
