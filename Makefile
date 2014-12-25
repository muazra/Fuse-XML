
LDIR =-lxml2 -lz -lm -lfuse
IDIR =/usr/include/libxml2
CC=gcc
CFLAGS=-I. -I$(IDIR) -D_FILE_OFFSET_BITS=64 


DEPS = xmlUtils.h
OBJ = fusexmp.o xmlUtils.o 

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $< 

fusexmp: $(OBJ)
	gcc -g -o $@ $^ $(CFLAGS) $(LDIR)

.PHONY: clean

clean:
	rm -f *.o *~ core fusexmp
