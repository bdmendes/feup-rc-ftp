CC = gcc 
CFLAGS = -Wall
PROG_NAME = download

SRCDIR = src

SOURCES := $(shell find $(SRCDIR) -name '*.c')
OBJECTS := $(SOURCES:%.c=%.o)

.PHONY: clean

all: download

download: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(PROG_NAME) $(OBJECTS)

clean:
	rm -f $(PROG_NAME) $(OBJECTS)
