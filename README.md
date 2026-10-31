<div align="center">

# TimSim

### A Clean, Fast, and Interactive Turing Machine Simulator for the Terminal

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
![C Standard](https://img.shields.io/badge/C-99-orange.svg)
![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)

</div>

---

**TimSim** is a lightweight, zero-dependency Turing machine simulator written in plain C. It is built for students and anyone studying the theory of computation: type a machine in (or load it from a file), TimSim validates it, and then runs it on any input with an optional step-by-step trace.

---

## Features

- **Three ways to get a machine:** type it in interactively, load a machine file, or try the built-in demo (`--demo`).
- **Strict validation:** every problem in a machine file is reported with its line number; non-deterministic rules (two transitions for the same state and symbol) and states that are both accepting and rejecting are refused. `--check` validates a file without running it.
- **Helpful warnings:** an accepting state nothing leads to, transitions out of a halting state (they can never run), and a start state without any transition.
- **Several ways to watch a run:** a result summary, a step-by-step trace (`-t`), a debugger-style walk with Enter between steps (`--step`), an animated tape (`--animate`), or just the verdict (`-q`).
- **Never hangs:** a step limit (1 to 100,000) and a fixed-size tape end a run with "no decision" instead of an endless loop.
- **Script friendly:** exit codes for accepted, rejected, error and no decision; plain output when piped; colour only on a terminal (`NO_COLOR` and `--color` respected).
- **Friendly interactive mode:** a bad line is rejected on the spot and only that line is re-entered; save the machine to a file; run as many inputs as you like on the same machine.
- **Multiple accepting and rejecting states** (up to 10 each) and up to 30 transitions.
- **Runs everywhere:** Linux, macOS, WSL and Windows, no dependencies, one-line installers, and a test suite (`make test`).

---

## Quick Installation

### Linux / macOS / WSL

Needs `git`, `make` and a C compiler (`cc`, `gcc` or `clang`); anything missing is installed with the system package manager. The installer builds from source and installs `timsim` to `/usr/local/bin` (it asks for `sudo` only if needed). The example machines are installed too, to `/usr/local/share/timsim/examples/`:

```sh
sh -c "$(curl -fsSL https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.sh)"
```

*Want to install without root/admin privileges?* Everything then goes under `~/.local` (examples in `~/.local/share/timsim/examples/`):

```sh
PREFIX=$HOME/.local sh -c "$(curl -fsSL https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.sh)"
```

Then try an example:

```sh
timsim /usr/local/share/timsim/examples/palindrome.tm -i 0110
```

### Windows (PowerShell)

Open **PowerShell** (no administrator rights needed) and run:

```powershell
irm https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.ps1 | iex
```

It downloads `timsim.exe` from the [latest release](https://github.com/krishnacode22/TimSim/releases/latest) into `%LOCALAPPDATA%\Programs\TimSim`, puts the example machines in its `examples\` folder, and adds the folder to your user `PATH`. Then type `timsim` in any terminal. If the command is not found, open a new terminal window.

```powershell
timsim "$env:LOCALAPPDATA\Programs\TimSim\examples\palindrome.tm" -i 0110
```

To remove it again:

```powershell
$env:TIMSIM_UNINSTALL=1; irm https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.ps1 | iex
```

You can also download `timsim.exe` from the Releases page and run it from anywhere. WSL users can follow the Linux steps instead.

### Updating

Run the install command again. It replaces the old version with the newest one.

---

## Usage

```sh
timsim                          # interactive: type the machine in
timsim machine.tm               # load a machine file, then ask for the input string
timsim machine.tm -i 1011       # run on 1011 and exit (no prompts, good for scripts)
timsim machine.tm -i 1011 -t    # same, with a step-by-step trace
timsim machine.tm -i 1011 --step # walk through it, Enter between steps
timsim machine.tm -i 1011 --animate 200   # redraw the tape as it runs
timsim --demo                   # built-in example with a trace
timsim --help                   # all options
```

### Command line options

| Option | What it does |
| --- | --- |
| `FILE` or `--run FILE` | Load a machine file |
| `-i`, `--input STRING` | Run on `STRING` and exit. Needs a file or `--demo` |
| `-t`, `--trace` | Show the step-by-step trace |
| `-l`, `--limit N` | Step limit, 1 to 100000. Overrides the file and the prompt |
| `-q`, `--quiet` | Only the essentials: with `--input` the verdict and the final tape; also hides the banner and summary |
| `--check` | Only validate `FILE`, do not run it |
| `--demo` | Run the built-in binary incrementer |
| `--step` | Pause between steps; press Enter to continue (implies `-t`) |
| `--animate MS` | Redraw the tape every `MS` ms, 10 to 2000 (implies `-t`; only on a terminal) |
| `--color=WHEN` | Colour: `auto` (default), `always`, `never` |
| `-v`, `--version` | Show the version |
| `-h`, `--help` | Show help |

`--animate` redraws the trace line in place, so the head looks like it moves along
the tape. It only animates when the output is a terminal; piped runs (and `-q`)
fall back to the normal trace, which keeps scripts and the test suite unchanged.
It can not be combined with `--step`.

Colour is used only when the output goes to a terminal, `NO_COLOR` is unset and
`TERM` is not `dumb`. `--color=always` forces it, `--color=never` turns it off, and
`-q` output is always plain, so piped and scripted output stays free of escape
codes. The verdict words (`ACCEPTED`, `REJECTED`) never depend on colour.

### Exit status with `--input`

| Code | Meaning |
| --- | --- |
| `0` | Accepted |
| `1` | Rejected (rejecting state, or no transition defined) |
| `2` | Error (bad option, bad machine file, bad input string) |
| `3` | No decision (step limit or tape boundary reached) |

`--check` exits with `0` for a valid machine and `2` for an invalid one.

With `--quiet` the verdict is one line: `ACCEPTED`, `REJECTED`, `STEP_LIMIT` or `TAPE_LIMIT`, followed by the final tape.

That makes TimSim easy to use in scripts:

```sh
if timsim examples/palindrome.tm -i 0110 -q > /dev/null; then echo "palindrome"; fi
timsim examples/binary_increment.tm -i 1011 -q     # prints ACCEPTED, then 1100
```

(Paths are relative to a cloned repository. For an installed copy, use the `share/timsim/examples/` folder shown above.)

### Interactive prompts

1. **Number of transitions** (1 to 30).
2. **Transition rules** in the format `{from} {to} {read} {write} {move}`, e.g. `0 1 a b R`. Mistakes (bad state, bad symbol, conflicting rule) are rejected right away and only that line is re-entered.
3. **Machine setup:** start state, accepting state(s), optional rejecting state(s), step limit.
4. **Save** the machine to a file, if you want to run it again later with `timsim FILE`.
5. **Run:** enter an input string and choose whether to show the trace. Run as many inputs as you like on the same machine.

---

## Machine files

Plain text, one item per line. Text after `#` is a comment.

```text
# Accepts binary strings with an even number of 0s
start 0              # start state
accept 2             # accepting state(s), separated by spaces
reject 3             # rejecting state(s), optional
limit 1000           # step limit, optional

0 1 0 0 R            # from to read write move
0 0 1 1 R
0 2 _ _ S
1 0 0 0 R
1 1 1 1 R
1 3 _ _ S
```

TimSim checks the whole file and reports every line-level problem with its line
number, then a summary:

```text
tests/bad/many_errors.tm:3: invalid direction 'X' (use L, R or S)
tests/bad/many_errors.tm:4: symbols and move must be one visible character each
tests/bad/many_errors.tm:5: states must be 0 to 99
tests/bad/many_errors.tm:6: unknown word 'bogus' (use start, accept, reject or limit)
tests/bad/many_errors.tm: no transitions found
tests/bad/many_errors.tm: 5 error(s) found, machine not loaded.
```

`start` and `accept` are required; `reject` and `limit` are optional. Every
`start`, `accept`, `reject` and `limit` line may appear only once.

Problems that belong to the whole file (a missing `start` line, no transitions)
have no line number. Warnings and the trace refer to transitions by their
position in the table (`transition 2`), not by their line in the file.

Beyond errors, TimSim prints **warnings** (the machine still runs) when an
accepting state is never used as a next state, when a transition starts from a
halting state (it can never be used), or when the start state has no transition:

```text
Warning: accepting state 1 is never used as a next state, it can never be reached.
```

---

## Rules

| Feature | Specification / Constraint |
| --- | --- |
| **States** | Whole numbers from `0` to `99` |
| **Tape symbols** | Any visible non-whitespace character |
| **Blank symbol** | `_` (cannot appear in the input string) |
| **Head movement** | `L` (Left), `R` (Right), or `S` (Stay), case-insensitive |
| **Determinism** | One transition per `(state, read symbol)`. A conflict is rejected with the lines involved |
| **Halting** | An accepting state accepts. A rejecting state, or a missing transition, rejects |
| **Step limit** | 1 to 100,000 (default 1,000). Hitting it means "no decision", not "loops forever" |
| **Tape** | Fixed size: 10,000 cells each side of the start. The run stops with a message at either edge |
| **Input string** | Up to 200 characters |
| **Size** | Up to 30 transitions, up to 10 accepting and 10 rejecting states |

A valid transition table does not prove that a machine is correct or that it halts.

---

## Example machines (`examples/`)

The example machines are in `examples/` in the repository, and the installers copy the `.tm` files to `share/timsim/examples/` (Linux/macOS) or `examples\` next to `timsim.exe` (Windows).

| File | Description | Try |
| --- | --- | --- |
| `binary_increment.tm` | Adds 1 to a binary number | `-i 1011` gives `1100` |
| `palindrome.tm` | Accepts binary palindromes | `-i 0110` accepted, `-i 0100` rejected |
| `even_zeros.tm` | Accepts an even number of 0s | `-i 1001` accepted, `-i 10` rejected |
| `loops_forever.tm` | Never halts, only the step limit stops it | `-i "" -l 50` hits the step limit |
| `binary_increment.txt` | Interactive answers for `binary_increment.tm`, accepted run | `timsim < examples/binary_increment.txt` |
| `even_zeros_accept.txt` | Interactive answers for `even_zeros.tm`, accepted run | `timsim < examples/even_zeros_accept.txt` |
| `even_zeros_reject.txt` | Interactive answers for `even_zeros.tm`, rejected run | `timsim < examples/even_zeros_reject.txt` |
| `step_limit.txt` | Interactive answers, halts at the step limit | `timsim < examples/step_limit.txt` |

---

## Build from source

```sh
git clone https://github.com/krishnacode22/TimSim.git
cd TimSim
make                                # build ./timsim
make test                           # run the test suite (no internet needed)
sudo make install                   # install to /usr/local
make install PREFIX=$HOME/.local    # or install without root
make windows                        # cross-compile timsim.exe (needs mingw-w64)
```

`make install` installs the program and the example machines. Uninstall with
`sudo make uninstall` (use the same `PREFIX` you installed with).

The build uses C99 with POSIX calls (`-std=gnu99`), so `cc`, `gcc` and `clang`
all work.

### Environment variables

| Variable | Used by | Meaning |
| --- | --- | --- |
| `NO_COLOR` | `timsim` | Non-empty value turns colour off (in `auto` mode) |
| `PREFIX` | `install.sh`, `make install` | Install folder (default `/usr/local`) |
| `TIMSIM_REPO` | `install.sh` | Git repository to build from |
| `TIMSIM_INSTALL_DIR` | `install.ps1` | Install folder on Windows |
| `TIMSIM_DOWNLOAD_URL` | `install.ps1` | Where `timsim.exe` is downloaded from |
| `TIMSIM_UNINSTALL` | `install.ps1` | `1` removes TimSim instead of installing |

**Releasing a new version:** change `VERSION`, run `make windows`, upload `timsim.exe`
to a new GitHub release, check it with `timsim.exe --version`, then push to `main`.
(Windows users get the latest release, Linux/macOS users get `main`, so publish the
`.exe` first.)

## License

Distributed under the **MIT License**. See `LICENSE` for more details.
