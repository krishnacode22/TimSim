# TimSim - Turing Machine Simulator
# Usage:
#   make                 build ./timsim
#   make test            build and run the test suite
#   sudo make install    install to /usr/local/bin
#   make install PREFIX=$HOME/.local   install without sudo
#   make uninstall       remove the installed program
#   make windows         cross-compile timsim.exe on Linux (needs mingw-w64)
#
# The version number lives in the VERSION file (the update checker compares it
# with the VERSION file on GitHub), and is compiled into the program.

CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra
# The POSIX calls (popen, isatty) need more than strict C99, so ask for gnu99.
STD      = -std=gnu99
PREFIX  ?= /usr/local
BINDIR   = $(DESTDIR)$(PREFIX)/bin
VERSION := $(shell tr -d ' \r\n' < VERSION)
WINCC   ?= x86_64-w64-mingw32-gcc

all: timsim

# The install prefix is compiled into the program (--update installs back into the
# same folder), so it is remembered in .prefix and a change forces a rebuild.
.prefix: FORCE
	@echo '$(PREFIX)' | cmp -s - $@ || echo '$(PREFIX)' > $@

FORCE:

timsim: timsim.c VERSION .prefix
	$(CC) $(CFLAGS) $(STD) -DTIMSIM_VERSION='"$(VERSION)"' -DTIMSIM_PREFIX='"$(PREFIX)"' -o timsim timsim.c

# Windows build: static, so the .exe needs no extra DLLs. Upload it to the GitHub release as "timsim.exe".
windows: timsim.exe

timsim.exe: timsim.c VERSION
	$(WINCC) $(CFLAGS) $(STD) -static -DTIMSIM_VERSION='"$(VERSION)"' -o timsim.exe timsim.c

test: timsim
	@sh tests/run_tests.sh ./timsim

install: timsim
	install -d $(BINDIR)
	install -m 755 timsim $(BINDIR)/timsim
	@echo "Installed to $(BINDIR)/timsim"

uninstall:
	rm -f $(BINDIR)/timsim
	@echo "Removed $(BINDIR)/timsim"

clean:
	rm -f timsim timsim.exe .prefix

.PHONY: all windows test install uninstall clean FORCE
