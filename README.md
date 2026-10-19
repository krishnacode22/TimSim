```markdown
<div align="center">

# TimSim
### A Clean, Fast, and Interactive Turing Machine Simulator for the Terminal

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE)
[![C Standard](https://img.shields.io/badge/C-99%2F11-orange.svg)](https://en.wikipedia.org/wiki/C99)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)]()

</div>

---

**TimSim** is a lightweight, zero-dependency, highly robust Turing machine simulator written in pure C. Designed for students, educators, and enthusiasts studying theory of computation, it features strict determinism validation, interactive step-by-step execution traces, dynamic tape cell expansion, and automated one-line installation across major platforms.

---

## Quick Installation

### Linux / macOS
Run the single-line installer below. It automatically checks for system prerequisites (`git`, `make`, and a C compiler) and installs **TimSim** globally:

```sh
sh -c "$(curl -fsSL [https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.sh](https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.sh))"

```

*Want to install without root/admin privileges?*

```sh
PREFIX=$HOME/.local sh -c "$(curl -fsSL [https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.sh](https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.sh))"

```

### Windows (PowerShell)

Open **PowerShell** as a standard user and execute the automated installer to download and configure TimSim instantly:

```powershell
irm [https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.ps1](https://raw.githubusercontent.com/krishnacode22/TimSim/main/install.ps1) | iex

```

---

## Usage

Once installed, simply type `timsim` in your terminal or use the file-runner flag:

```bash
timsim

```

Alternatively, feed a pre-configured machine file directly into the simulator using input redirection or the flag:

```bash
timsim < examples/binary_increment.txt
# OR
timsim --run examples/binary_increment.txt

```

### Interactive Prompts Workflow:

1. **Number of Transitions:** Specify how many rules your machine requires (1 to 30).
2. **Transition Rules:** Enter each rule following the format: `{from_state} {to_state} {read_symbol} {write_symbol} {move}` (e.g., `0 1 a b R`).
3. **Machine Setup:** Enter your designated start state, space-separated accepting state(s), optional rejecting state(s), and safety step limit.
4. **Execution:** Input your target test string and choose whether to render a live step-by-step trace showing the tape head position `[x]`.

---

## Formal Specifications & Rules

| Feature | Specification / Constraint |
| --- | --- |
| **States ($Q$)** | Whole numbers ranging from `0` to `99`. |
| **Tape Symbols ($\Gamma$)** | Any visible non-whitespace character. |
| **Blank Symbol ($B$)** | Designated as `_` (cannot appear inside input strings). |
| **Head Movement** | `L` (Left), `R` (Right), or `S` (Stay) — case-insensitive. |
| **Determinism Check** | Strictly enforced; duplicate entries for a given `(state, read_symbol)` trigger conflict warnings immediately during input. |
| **Halting Logic** | Reaching an accepting state accepts; reaching a rejecting state or encountering an undefined transition configuration rejects. |
| **Step Safeguard** | Configurable from 1 to 100,000 steps (default: 1,000) to prevent infinite loops. |
| **Tape Capacity** | Dynamically padded with 10,000 blank cells on both sides of the start head. |

---

## Sample Machines (`examples/`)

TimSim comes packed with pre-tested machine models inside the `examples/` directory:

| File Name | Description | Test Input | Expected Result |
| --- | --- | --- | --- |
| `binary_increment.txt` | Binary incrementer ($x + 1$) | `1011` | Accepted, tape outputs `1100` |
| `even_zeros_accept.txt` | Parity checker for zeros | `1001` | Accepted (even zeros) |
| `even_zeros_reject.txt` | Parity checker for zeros | `10` | Rejected (odd zeros) |
| `step_limit.txt` | Infinite-right mover | `_` | Halted: Step limit reached |

---

## 🛠️ Manual Build & Development

If you prefer cloning and compiling the source repository manually:

```bash
# Clone the repository
git clone [https://github.com/krishnacode22/TimSim.git](https://github.com/krishnacode22/TimSim.git)
cd TimSim

# Build binary locally
make

# Run automated sample test suites
make test

# Install system-wide
sudo make install

```

### Uninstallation

To cleanly remove TimSim from your system:

```bash
sudo make uninstall

```

## License

Distributed under the **MIT License**. See `LICENSE` for more details.

```

```
