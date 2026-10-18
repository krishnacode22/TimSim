# TimSim

A beginner-friendly **Turing machine simulator** for the terminal, written in plain C (single file, no dependencies).

You enter the transition rules, TimSim validates them, and then runs the machine on any input string, with an optional step-by-step trace.

## Install (Linux / macOS)

```sh
sh -c "$(curl -fsSL https://raw.githubusercontent.com/YOUR_GITHUB_USERNAME/timsim/main/install.sh)"
```

Needs `git`, `make` and a C compiler (`cc`, `gcc` or `clang`). It builds from source and installs `timsim` to `/usr/local/bin` (asks for `sudo` only if needed).

Install without admin rights:

```sh
PREFIX=$HOME/.local sh -c "$(curl -fsSL https://raw.githubusercontent.com/YOUR_GITHUB_USERNAME/timsim/main/install.sh)"
```

Windows: use WSL.

### Build manually

```sh
git clone https://github.com/YOUR_GITHUB_USERNAME/timsim.git
cd timsim
make            # builds ./timsim
make test       # runs the sample machines
sudo make install
```

### Uninstall

```sh
sudo make uninstall      # or: sudo rm /usr/local/bin/timsim
```

## Usage

Run `timsim` and answer the prompts:

1. Number of transitions (1 to 30)
2. Each transition as `{from} {to} {read} {write} {move}`, e.g. `0 1 a b R`
3. Start state, accepting state(s), optional rejecting state(s), step limit
4. Input string, and whether to show the trace

Machine files can be piped in, so a machine is repeatable:

```sh
timsim < examples/binary_increment.txt
```

### Rules

| Item | Rule |
|---|---|
| States | whole numbers `0` to `99` |
| Tape symbols | any visible character, no spaces |
| Blank symbol | `_` (cannot appear in the input string) |
| Moves | `L`, `R`, `S` (lowercase accepted) |
| Determinism | one transition per (state, read symbol); conflicts are rejected |
| Halting | accepting state = accept, rejecting state = reject, no matching transition = reject |
| Step limit | 1 to 100000 (default 1000). Hitting it means "no decision", not "loops forever" |
| Tape | 10000 cells each side of the start position |

A valid transition table does not prove the machine is correct or that it halts.

### Sample machines (`examples/`)

| File | Machine | Input | Expected |
|---|---|---|---|
| `binary_increment.txt` | adds 1 to a binary number | `1011` | accepted, tape `1100` |
| `even_zeros_accept.txt` | accepts strings with an even number of 0s | `1001` | accepted |
| `even_zeros_reject.txt` | same machine | `10` | rejected |
| `step_limit.txt` | runs right forever | (empty) | step limit reached |

## Project status

- [x] Phase 1: transition input
- [x] Phase 2: validation and conflict detection
- [x] Phase 3: machine execution
- [ ] Phase 4: UI and release

## License

MIT, see `LICENSE`.
