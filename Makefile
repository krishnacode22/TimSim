# TimSim - Turing Machine Simulator
# Usage:
#   make                 build ./timsim
#   make test            build and run the test suite
#   sudo make install    install to /usr/local (program + example machines)
#   make install PREFIX=$HOME/.local   install without sudo
#   make uninstall       remove the installed program and examples
#   make windows         cross-compile timsim.exe on Linux (needs mingw-w64)
#
# The version number lives in the VERSION file and is compiled into the program.

CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra
# nanosleep (used by --animate) needs more than strict C99, so ask for gnu99.
STD      = -std=gnu99
PREFIX  ?= /usr/local
BINDIR   = $(DESTDIR)$(PREFIX)/bin
EXDIR    = $(DESTDIR)$(PREFIX)/share/timsim/examples
VERSION := $(shell tr -d ' \r\n' < VERSION)
WINCC   ?= x86_64-w64-mingw32-gcc

all: timsim

timsim: timsim.c VERSION
	$(CC) $(CFLAGS) $(STD) -DTIMSIM_VERSION='"$(VERSION)"' -o timsim timsim.c

# Windows build: static, so the .exe needs no extra DLLs. Upload it to the GitHub release as "timsim.exe".
windows: timsim.exe

timsim.exe: timsim.c VERSION
	$(WINCC) $(CFLAGS) $(STD) -static -DTIMSIM_VERSION='"$(VERSION)"' -o timsim.exe timsim.c

test: timsim
	@sh tests/run_tests.sh ./timsim

install: timsim
	install -d $(BINDIR) $(EXDIR)
	install -m 755 timsim $(BINDIR)/timsim
	install -m 644 examples/*.tm $(EXDIR)/
	@echo "Installed to $(BINDIR)/timsim"
	@echo "Example machines in $(EXDIR)"

uninstall:
	rm -f $(BINDIR)/timsim
	rm -rf $(DESTDIR)$(PREFIX)/share/timsim
	@echo "Removed $(BINDIR)/timsim and $(DESTDIR)$(PREFIX)/share/timsim"

clean:
	rm -f timsim timsim.exe

.PHONY: all windows test install uninstall clean
