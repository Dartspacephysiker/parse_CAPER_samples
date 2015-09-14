# ---------------------------------------------------------------------------
# Makefile - Please don't hurt 'em, Hammer
# ---------------------------------------------------------------------------

SRC_DIR=./
CFLAGS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -ggdb -fpack-struct=1

#CFLAGS=-iquote../../irig106lib/src -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -ggdb -fpack-struct=1
#LIBS=../../irig106lib/gcc/libirig106.a

EXEC= parse_CAPER_samples
#EXEC_23 = parse_TM23_samples

SRC = parse_CAPER_samples.c
#SRC_23 = parse_TM23_samples.c

OBJ = $(SRC:.c=.o)
#OBJ_23 = $(SRC_23:.c=.o)

HEADERS = TM1_majorframe.h
#HEADERS_23 = TM23_majorframe.h

#all: $(SRC) $(EXEC) $(SRC_23) $(EXEC_23)
all: $(SRC) $(EXEC)

$(EXEC): $(OBJ)
	gcc $(CFLAGS) $< -o $@

#$(EXEC_23): $(OBJ_23)
#	gcc $(CFLAGS) $< -o $@

#.c.o: $(HEADERS) $(HEADERS_23)
.c.o: $(HEADERS) $(HEADERS_23)
	$(CC) -o $@ $(CFLAGS) -c $<

.PHONY: clean

clean:
	rm parse_CAPER_samples *.o
#	rm parse_CAPER_samples parse_TM23_samples *.o
