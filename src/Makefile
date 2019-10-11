CC := gcc
CFLAGS :=
INSTALL := install
PREFIX := /usr/local
bindir := $(PREFIX)/bin

lunar_menu: main.c llst_gen.c util.c parse_menu.c hit_map.c encode_menu.c llst_gen.h util.h parse_menu.h hit_map.h encode_menu.h
	$(CC) $(CFLAGS) -Wall main.c llst_gen.c util.c parse_menu.c hit_map.c encode_menu.c -o $@

.PHONY: all clean install

all: lunar_menu

install: lunar_menu
	$(INSTALL) -d $(bindir)
	$(INSTALL) lunar_menu $(bindir)

clean:
	rm -f lunar_menu
