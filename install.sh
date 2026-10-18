#!/bin/sh
# TimSim installer.  Works with plain `sh` (no bash needed).
#
#   sh -c "$(curl -fsSL https://raw.githubusercontent.com/YOUR_GITHUB_USERNAME/timsim/main/install.sh)"
#
# Optional environment variables:
#   PREFIX=$HOME/.local   install somewhere else (no sudo needed if writable)
#   TIMSIM_REPO=<url>     clone from a different repository (forks, testing)
set -e

REPO_URL="${TIMSIM_REPO:-https://github.com/YOUR_GITHUB_USERNAME/timsim.git}"
PREFIX="${PREFIX:-/usr/local}"

echo "========================================"
echo "   Installing TimSim..."
echo "========================================"

# Check the tools we need
for tool in git make; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "Error: '$tool' is not installed. Please install it and run this again."
        exit 1
    fi
done
if ! command -v cc >/dev/null 2>&1 && ! command -v gcc >/dev/null 2>&1 && ! command -v clang >/dev/null 2>&1; then
    echo "Error: no C compiler found (cc, gcc or clang). Please install one and try again."
    exit 1
fi

# Clone into a temporary directory that is always cleaned up
TEMP_DIR=$(mktemp -d)
trap 'rm -rf "$TEMP_DIR"' EXIT INT TERM

echo "Downloading source..."
if ! git clone --quiet --depth 1 "$REPO_URL" "$TEMP_DIR/timsim"; then
    echo "Error: could not download TimSim from $REPO_URL"
    exit 1
fi

cd "$TEMP_DIR/timsim"

echo "Building..."
if ! make >/dev/null; then
    echo "Error: build failed. Run 'make' inside the repository to see the full error."
    exit 1
fi

# Use sudo only when the install folder is not writable by the current user
echo "Installing to $PREFIX/bin ..."
if mkdir -p "$PREFIX/bin" 2>/dev/null && [ -w "$PREFIX/bin" ]; then
    make install PREFIX="$PREFIX" >/dev/null
else
    if ! command -v sudo >/dev/null 2>&1; then
        echo "Error: $PREFIX/bin is not writable and 'sudo' is not available."
        echo "Try again with a folder you own, e.g.:  PREFIX=\$HOME/.local sh install.sh"
        exit 1
    fi
    echo "(administrator permission is needed to write to $PREFIX/bin)"
    sudo make install PREFIX="$PREFIX" >/dev/null
fi

echo "========================================"
echo " TimSim installed successfully!"
echo " Type 'timsim' anywhere to run it."
echo " To remove it later:  sudo rm $PREFIX/bin/timsim"
echo "========================================"

case ":$PATH:" in
    *":$PREFIX/bin:"*) ;;
    *) echo "Note: $PREFIX/bin is not in your PATH. Add this to your shell profile:"
       echo "  export PATH=\"$PREFIX/bin:\$PATH\"" ;;
esac
