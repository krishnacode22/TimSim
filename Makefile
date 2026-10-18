# TimSim - Turing Machine Simulator
# Usage:
#   make                 build ./timsim
#   make test            build and run the sample machines
#   sudo make install    install to /usr/local/bin
#   make install PREFIX=$HOME/.local   install without sudo
#   make uninstall       remove the installed program

CC      ?= cc
CFLAGS  ?= -O2 -Wall -Wextra
PREFIX  ?= /usr/local
BINDIR   = $(DESTDIR)$(PREFIX)/bin

all: timsim

timsim: timsim.c
	$(CC) $(CFLAGS) -o timsim timsim.c

test: timsim
	@echo "Running sample machines..."
	@./timsim < examples/binary_increment.txt | grep -q "RESULT: ACCEPTED" && \
	 ./timsim < examples/binary_increment.txt | grep -q "Final tape  : 1100" && \
	 echo "  PASS  binary_increment (1011 -> 1100)"
	@./timsim < examples/even_zeros_accept.txt | grep -q "RESULT: ACCEPTED" && \
	 echo "  PASS  even_zeros_accept (1001 accepted)"
	@./timsim < examples/even_zeros_reject.txt | grep -q "RESULT: REJECTED" && \
	 echo "  PASS  even_zeros_reject (10 rejected)"
	@./timsim < examples/step_limit.txt | grep -q "RESULT: STEP LIMIT REACHED" && \
	 echo "  PASS  step_limit (infinite loop stopped)"
	@echo "All tests passed."

install: timsim
	install -d $(BINDIR)
	install -m 755 timsim $(BINDIR)/timsim
	@echo "Installed to $(BINDIR)/timsim"

uninstall:
	rm -f $(BINDIR)/timsim
	@echo "Removed $(BINDIR)/timsim"

clean:
	rm -f timsim

.PHONY: all test install uninstall clean
