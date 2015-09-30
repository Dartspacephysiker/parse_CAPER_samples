# ---------------------------------------------------------------------------
# Makefile - Please don't hurt 'em, Hammer
# ---------------------------------------------------------------------------

SRC_DIR=./
CFLAGS=-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE -ggdb -fpack-struct=1
LDFLAGS = -pipe -Wall

CC = gcc

EXEC= parse_PCM_samples
SRC = parse_PCM_samples.c
OBJ = $(SRC:.c=.o)
HEADERS = parse_PCM_samples.h PCM_and_measurement_structs.h defaults.h TM1_majorframe.h TM23_majorframe.h ascii_reader.h

all: $(SRC) $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $(LDFLAGS) $?

.c.o: $(HEADERS)
	$(CC) -o $@ $(CFLAGS) -c $?

.PHONY: clean

clean:
	rm parse_PCM_samples *.o 
