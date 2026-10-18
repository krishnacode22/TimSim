#!/bin/sh
# TimSim automated installer. 
# Works with plain `sh` and auto-installs missing dependencies.
set -e

REPO_URL="${TIMSIM_REPO:-https://github.com/krishnacode22/TimSim.git}"
PREFIX="${PREFIX:-/usr/local}"

echo "========================================"
echo "   Installing TimSim & Dependencies..."
echo "========================================"

# Function to check and auto-install missing tools
install_dependencies() {
    MISSING_TOOLS=""
    
    if ! command -v git >/dev/null 2>&1; then MISSING_TOOLS="$MISSING_TOOLS git"; fi
    if ! command -v make >/dev/null 2>&1; then MISSING_TOOLS="$MISSING_TOOLS make"; fi
    if ! command -v cc >/dev/null 2>&1 && ! command -v gcc >/dev/null 2>&1 && ! command -v clang >/dev/null 2>&1; then
        MISSING_TOOLS="$MISSING_TOOLS compiler"
    fi

    if [ -n "$MISSING_TOOLS" ]; then
        echo "Missing required tools:$MISSING_TOOLS. Installing them automatically..."
        
        # Check if sudo is available or if we are root
        SUDO=""
        if [ "$(id -u)" -ne 0 ]; then
            if command -v sudo >/dev/null 2>&1; then
                SUDO="sudo"
            else
                echo "Error: Missing dependencies ($MISSING_TOOLS) and 'sudo' is not available to install them."
                echo "Please install git, make, and a C compiler manually and run this script again."
                exit 1
            fi
        fi

        # Detect package manager and install dependencies
        if command -v apt-get >/dev/null 2>&1; then
            $SUDO apt-get update &&$SUDO apt-get install -y git make build-essential
        elif command -v dnf >/dev/null 2>&1; then
            $SUDO dnf install -y git make gcc
        elif command -v pacman >/dev/null 2>&1; then
            $SUDO pacman -S --noconfirm git make gcc
        elif command -v apk >/dev/null 2>&1; then
            $SUDO apk add --no-cache git make build-base
        elif command -v brew >/dev/null 2>&1; then
            brew install git make
        else
            echo "Error: Could not detect a supported package manager (apt, dnf, pacman, apk, brew)."
            echo "Please manually install: git, make, and a C compiler."
            exit 1
        fi
    fi
}

# Run dependency check and installation
install_dependencies

# Clone into a temporary directory that is always cleaned up
TEMP_DIR=$(mktemp -d)
trap 'rm -rf "$TEMP_DIR"' EXIT INT TERM

echo "Downloading source..."
if ! git clone --quiet --depth 1 "$REPO_URL" "$TEMP_DIR/timsim"; then
    echo "Error: could not download TimSim from $REPO_URL"
    exit 1
fi

cd "$TEMP_DIR/timsim"

echo "Building TimSim..."
if ! make >/dev/null; then
    echo "Error: build failed. Run 'make' inside the repository to see the full error."
    exit 1
fi

# Use sudo only when the install folder is not writable by the current user
echo "Installing to $PREFIX/bin ..."
if mkdir -p "$PREFIX/bin" 2>/dev/null && [ -w "$PREFIX/bin" ]; then
    make install PREFIX="$PREFIX" >/dev/null
else
    if [ -z "$SUDO" ] && ! command -v sudo >/dev/null 2>&1; then
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
