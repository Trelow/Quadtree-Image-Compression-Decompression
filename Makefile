CC=gcc
CFLAGS=-Wall -Wextra -Werror -g
LDFLAGS=-lm

build: quadtree

quadtree: quadtree.c quadtree.h
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

run: quadtree
	./quadtree

clean:
	rm -f quadtree
