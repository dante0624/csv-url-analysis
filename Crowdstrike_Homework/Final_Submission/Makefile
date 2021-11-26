CC=cc
CFLAGS=-g -Wall -Werror -std=gnu99
TARGETS=server solution

all: $(TARGETS)

$(TARGETS): % : %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -rf *.o $(TARGETS)
