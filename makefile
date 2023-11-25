CC := gcc

FLAGS := -Wall -O3 -march=native
LIBS := -lavformat -lavcodec -lavutil

all: mp2a

mp2a: main.c
	$(CC) -o $@ $^ $(FLAGS) $(LIBS)

install:
	cp main ~/.local/bin

uninstall:
	rm ~/.local/bin/mp2a

clean:
	rm -rf *~ mp2a

.PHONY: format clean install uninstall
