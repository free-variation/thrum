CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lasound -lm

SRCDIR = src/c
BINDIR = bin

all: $(BINDIR)/sine

$(BINDIR)/sine: $(SRCDIR)/sine.c
	mkdir -p $(BINDIR)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f $(BINDIR)/sine

.PHONY: all clean
