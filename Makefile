CC = gcc
EFLAGS = -g -Wall
DFLAGS = -shared -fPIC
SOURCES= $(wildcard *.c)
OBJECTS= $(SOURCES:.c=.o)

all: sincro.o
	$(CC) $(EFLAGS) -o sincro.out sincro.c -L. -ldl 

sincro.o:sincro.c copia.o copiammap.o
	$(CC) -c -fPIC copia.c
	$(CC) -c -fPIC copiammap.c
	$(CC) $(DFLAGS) -o libcopiaByte.so copia.o
	$(CC) $(DFLAGS) -o libcopiaMap.so copiammap.o

copia.o: copia.c copia.h
	$(CC) $(EFLAGS) -c  copia.c -ldl

copiammap.o: copiammap.c copia.h
	$(CC) $(EFLAGS) -c  copiammap.c -ldl

.PHONY : clean

clean:
	-rm -f $(TARGETS) $(OBJECTS)
	-rm -f *.so
	-rm -f *.out
