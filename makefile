IDIR =./include
CC=gcc
CFLAGS=-I $(IDIR)
OUTDIR=./

ODIR=src
TESTFILE=./src/test.txt

_DEPS = readbits.h 
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = readbits.o readbits.test.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OUTDIR)readbits-test: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)
	cp $(TESTFILE) ./

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
