MAINFILE := main
BINFILE := app
INCFOLDER := inc
SRCFOLDER := src
OBJFOLDER := obj

CC := gcc
LF := -lwiringPi
CFLAGS := -Wextra -Wall -pedantic
SRCFILES := $(wildcard $(SRCFOLDER)/*.c)

all: $(SRCFILES:$(SRCFOLDER)/%.c=$(OBJFOLDER)/%.o)
	$(CC) $(CFLAGS) obj/*.o -o $(BINFILE) $(LF)

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@ -I./inc $(LF)
	$(CC) $(CFLAGS) -c $(MAINFILE).c -o $(OBJFOLDER)/$(MAINFILE).o $(LF)

run: $(BINFILE)
	./$(BINFILE)

.PHONY: clean
clean:
	rm -rf obj/*
	rm -rf $(BINFILE)
