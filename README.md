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

## Quick Installation

### Linux / macOS / WSL

Needs `git`, `make` and a C compiler (`cc`, `gcc` or `clang`); anything missing is installed with the system package manager. The installer builds from source and installs `timsim` to `/usr/local/bin` (it asks for `sudo` only if needed):

```sh
sh -c "$(curl -fsSL https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.sh)"
```

*Want to install without root/admin privileges?*

```sh
PREFIX=$HOME/.local sh -c "$(curl -fsSL https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.sh)"
```

### Windows (PowerShell)

Open **PowerShell** (no administrator rights needed) and run:

```powershell
irm https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.ps1 | iex
```

It downloads `timsim.exe` from the [latest release](https://github.com/krishnacode22/TimSim/releases/latest) into `%LOCALAPPDATA%\Programs\TimSim` and adds that folder to your user `PATH`. Then type `timsim` in any terminal. If the command is not found, open a new terminal window.

To remove it again:

```powershell
$env:TIMSIM_UNINSTALL=1; irm https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.ps1 | iex
```

You can also download `timsim.exe` from the Releases page and run it from anywhere. WSL users can follow the Linux steps instead.

---

## Usage

```sh
timsim                          # interactive: type the machine in
timsim machine.tm               # load a machine file, then ask for the input string
timsim machine.tm -i 1011       # run on 1011 and exit (no prompts, good for scripts)
timsim machine.tm -i 1011 -t    # same, with a step-by-step trace
timsim machine.tm -i 1011 --step # walk through it, Enter between steps
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
| `--check-update` | Check GitHub for a newer version |
| `--update` | Install the newest version |
| `--no-update-check` | Skip the automatic daily update check |
| `--color=WHEN` | Colour: `auto` (default), `always`, `never` |
| `-v`, `--version` | Show the version |
| `-h`, `--help` | Show help |

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

With `--quiet` the verdict is one line: `ACCEPTED`, `REJECTED`, `STEP_LIMIT` or `TAPE_LIMIT`, followed by the final tape.

That makes TimSim easy to use in scripts:

```sh
if timsim examples/palindrome.tm -i 0110 -q > /dev/null; then echo "palindrome"; fi
timsim examples/binary_increment.tm -i 1011 -q     # prints ACCEPTED, then 1100
```

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

Problems that belong to the whole file (a missing `start` line, no transitions)
have no line number. Warnings and the trace refer to transitions by their
position in the table (`transition 2`), not by their line in the file.

---

## Updates

- Once a day, when you start TimSim in a terminal, it asks GitHub for the newest `VERSION` file. If a newer version exists you are asked: **[y]es / [n]o / [s]kip this version**.
- Nothing is installed without your answer. The check never runs in scripts (`--input`, `--check`, pipes) or with `--demo`, and never blocks for more than about 3 seconds (it uses one tool: `curl`, or `wget` when `curl` is missing).
- `timsim --check-update` only reports. `timsim --update` runs the official installer again: `install.sh` on Linux/macOS (into the same folder TimSim was installed in) and `install.ps1` on Windows (which downloads the newest `timsim.exe` from the releases page).
- Turn the automatic check off with `--no-update-check` or `TIMSIM_NO_UPDATE_CHECK=1` (`export TIMSIM_NO_UPDATE_CHECK=1` on Linux/macOS, `$env:TIMSIM_NO_UPDATE_CHECK=1` in PowerShell).
- It needs `curl` or `wget` (on Windows, the `curl.exe` that comes with Windows 10 and later). Without them the check is silently skipped.

| Command | Exit status |
| --- | --- |
| `timsim --check` | `0` valid machine, `2` invalid |
| `timsim --check-update` | `0` up to date, `1` could not check, `4` a newer version exists |
| `timsim --update` | `0` installed (or already current), `1` failed |

**Releasing a new version** (the order matters, because Windows downloads the
newest release while Linux builds `main`):

1. Change `VERSION` in your working copy.
2. Run `make windows` and upload `timsim.exe` to the new GitHub release.
3. Check the uploaded file with `timsim.exe --version`.
4. Only then push the `VERSION` change to `main`.

### Environment variables

| Variable | Used by | Meaning |
| --- | --- | --- |
| `TIMSIM_NO_UPDATE_CHECK` | `timsim` | Set to any value to skip the automatic update check |
| `NO_COLOR` | `timsim` | Non-empty value turns colour off (in `auto` mode) |
| `TIMSIM_VERSION_URL` | `timsim` | Where the version is read from (test hook) |
| `TIMSIM_INSTALL_URL` | `timsim` | Where the installer is read from (test hook) |
| `TIMSIM_REPO` | `install.sh` | Git repository to build from |
| `PREFIX` | `install.sh`, `make install` | Install folder (default `/usr/local`) |
| `TIMSIM_INSTALL_DIR` | `install.ps1` | Install folder on Windows |
| `TIMSIM_DOWNLOAD_URL` | `install.ps1` | Where `timsim.exe` is downloaded from |
| `TIMSIM_UNINSTALL` | `install.ps1` | `1` removes TimSim instead of installing |

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

A valid transition table does not prove that a machine is correct or that it halts.

---

## Example machines (`examples/`)

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
sudo make install                   # install to /usr/local/bin
make install PREFIX=$HOME/.local    # or install without root
make windows                        # cross-compile timsim.exe (needs mingw-w64)
```

Uninstall with `sudo make uninstall` (or `sudo rm /usr/local/bin/timsim`). The
`examples/` folder only exists in the repository, it is not installed.

The build uses C99 with POSIX calls (`-std=gnu99`), so `cc`, `gcc` and `clang`
all work. The install prefix is compiled into the program, so a change of
`PREFIX` triggers a rebuild.


## License

Distributed under the **MIT License**. See `LICENSE` for more details.
