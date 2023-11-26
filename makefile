CC := gcc

SOURCE := ./src
DIST := ./dist
INCLUDE := ./include

FLAGS := -Wall -O3 -march=native -I$(INCLUDE)
LIBS := -lavformat -lavcodec -lavutil -lSDL2 -lswresample

OBJECTS := $(DIST)/main.o $(DIST)/video.o $(DIST)/audio.o $(DIST)/screen.o
TARGET := mp2a

all: $(DIST) $(OBJECTS) $(TARGET)

$(DIST):
	mkdir -p $@

$(DIST)/%.o: $(SOURCE)/%.c $(INCLUDE)/%.h
	$(CC) -c -o $@ $< $(FLAGS) $(LIBS)

$(TARGET): $(OBJECTS)
	$(CC) -o $@ $^ $(FLAGS) $(LIBS)

install:
	cp $(TARGET) ~/.local/bin

uninstall:
	rm ~/.local/bin/$(TARGET)

clean:
	rm -rf *~ $(TARGET) $(DIST)

.PHONY: format clean install uninstall
