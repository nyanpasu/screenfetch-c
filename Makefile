CC=gcc
CFLAGS=-O3 -std=c99 -Wall
LDFLAGS_OSX=-lpthread
LDFLAGS_BSD=-lpthread
LDFLAGS_LINUX=-lpthread -lX11
LDFLAGS_SOLARIS=-lpthread -lX11
INSTALL=/usr/bin/install -c

BINDIR=/usr/local/bin
MANDIR=/usr/local/share/man/man1

SOURCES=screenfetch.c thread.c resources.c detect.c

all:
	@echo '========================================================='
	@echo 'You must run make with a target corresponding to your OS.'
	@echo 'Options: linux, solaris, bsd, osx, win.'
	@echo '========================================================='

linux:
	$(CC) $(CFLAGS) $(SOURCES) -o ./screenfetch-c $(LDFLAGS_LINUX)

solaris:
	$(CC) $(CFLAGS) $(SOURCES) -o ./screenfetch-c $(LDFLAGS_SOLARIS)

bsd:
	$(CC) $(CFLAGS) $(SOURCES) -o ./screenfetch-c $(LDFLAGS_BSD)

osx:
	$(CC) $(CFLAGS) $(SOURCES) -o ./screenfetch-c $(LDFLAGS_OSX)

win:
	$(CC) $(CFLAGS) $(SOURCES) -o ./screenfetch-c

install:
	$(INSTALL) screenfetch-c $(BINDIR)/screenfetch-c
	mkdir -p $(MANDIR)
	$(INSTALL) ./manpage/screenfetch-c.1 $(MANDIR)/screenfetch-c.1

uninstall:
	rm -rf $(BINDIR)/screenfetch-c
	rm -rf $(MANDIR)/screenfetch-c.1

clean:
	rm screenfetch-c
