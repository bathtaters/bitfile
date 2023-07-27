IDIR =./include
CC=gcc
CFLAGS=-g -Wall -I $(IDIR)
OUTDIR=./

ODIR=src
TESTFILE=./src/test.txt

_DEPS = bitfile.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = bitfile.o bitfile.test.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTDIR)bitfile-test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	cp $(TESTFILE) ./

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
