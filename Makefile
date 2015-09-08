# ---------------------------------------------------------------------------
# Makefile - Please don't hurt 'em, Hammer
# ---------------------------------------------------------------------------

SRC_DIR=./
CFLAGS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -ggdb -fpack-struct=1

#CFLAGS=-iquote../../irig106lib/src -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -ggdb -fpack-struct=1
#LIBS=../../irig106lib/gcc/libirig106.a

EXEC= parse_CAPER_samples
SRC = parse_CAPER_samples.c
OBJ = $(SRC:.c=.o)
HEADERS = TM1_majorframe.h

all: $(SRC) $(EXEC)

$(EXEC): $(OBJ)
	gcc $(CFLAGS) $< -o $@

.c.o: $(HEADERS)
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean

clean:
	rm parse_CAPER_samples
