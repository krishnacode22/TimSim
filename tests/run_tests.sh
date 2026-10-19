#!/bin/sh
# TimSim test suite.  Usage: sh tests/run_tests.sh ./timsim   (or: make test)
# Update-check tests use local file:// URLs, so no internet is needed.
BIN=$(cd "$(dirname "$1")" && pwd)/$(basename "${1:-timsim}")
cd "$(dirname "$0")/.." || exit 1
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
export TIMSIM_NO_UPDATE_CHECK=1
pass=0
fail=0

ok()   { pass=$((pass + 1)); echo "  PASS  $1"; }
bad()  { fail=$((fail + 1)); echo "  FAIL  $1"; }
# expect NAME EXPECTED_EXIT EXPECTED_TEXT COMMAND...   (text may be empty)
expect() {
    name=$1; want=$2; text=$3; shift 3
    out=$("$@" 2>&1); code=$?
    if [ "$code" -ne "$want" ]; then bad "$name (exit $code, wanted $want)"; return; fi
    if [ -n "$text" ] && ! printf '%s\n' "$out" | grep -qF -- "$text"; then bad "$name (output missing: $text)"; return; fi
    ok "$name"
}

echo "Command line"
expect "version"                 0 "timsim $(tr -d ' \r\n' < VERSION)" "$BIN" --version
expect "help"                    0 "Usage:"                    "$BIN" -h
expect "unknown option"          2 "unknown option"            "$BIN" --nope
expect "missing option value"    2 "needs a value"             "$BIN" -i
expect "bad step limit"          2 "step limit must be"        "$BIN" examples/palindrome.tm -i 1 -l 0
expect "two machine files"       2 "only one machine file"     "$BIN" a.tm b.tm
expect "--input without machine" 2 "needs a machine file"      "$BIN" -i 101
expect "missing file"            2 "cannot open"               "$BIN" does_not_exist.tm -i 1

echo "Running machines"
expect "increment 1011 -> 1100"  0 "1100"          "$BIN" examples/binary_increment.tm -i 1011 -q
expect "increment 111 -> 1000"   0 "1000"          "$BIN" examples/binary_increment.tm -i 111 -q
expect "palindrome accepts 0110" 0 "ACCEPTED"      "$BIN" examples/palindrome.tm -i 0110 -q
expect "palindrome accepts 101"  0 "ACCEPTED"      "$BIN" examples/palindrome.tm -i 101 -q
expect "palindrome accepts empty" 0 "ACCEPTED"     "$BIN" examples/palindrome.tm -i "" -q
expect "palindrome rejects 0100" 1 "REJECTED"      "$BIN" examples/palindrome.tm -i 0100 -q
expect "even zeros accepts 1001" 0 "ACCEPTED"      "$BIN" examples/even_zeros.tm -i 1001 -q
expect "even zeros rejects 10"   1 "REJECTED"      "$BIN" examples/even_zeros.tm -i 10 -q
expect "step limit exit 3"       3 "STEP_LIMIT"    "$BIN" examples/loops_forever.tm -i "" -l 50 -q
expect "--limit overrides"       3 "STEP_LIMIT"    "$BIN" examples/binary_increment.tm -i 1011 -l 3 -q
expect "trace output"            1 "Execution trace" "$BIN" examples/even_zeros.tm -i 10 -t
expect "full report"             0 "Final tape  : 1100" "$BIN" examples/binary_increment.tm -i 1011
expect "bad input char"          2 "invalid character" "$BIN" examples/even_zeros.tm -i "1 0"
expect "blank in input"          2 "invalid character" "$BIN" examples/even_zeros.tm -i "1_0"
expect "demo"                    0 "RESULT: ACCEPTED"  "$BIN" --demo
expect "demo custom input"       0 "10000"             "$BIN" --demo -i 1111 -q

echo "Machine file validation"
expect "--check valid file"      0 "valid deterministic" "$BIN" examples/palindrome.tm --check
expect "conflict reported"       2 "conflict - line 4"   "$BIN" tests/bad/conflict.tm --check
expect "missing start"           2 "missing 'start'"     "$BIN" tests/bad/missing_start.tm --check
expect "all errors listed"       2 "unknown word 'bogus'" "$BIN" tests/bad/many_errors.tm --check
expect "invalid move"            2 "invalid direction"   "$BIN" tests/bad/many_errors.tm --check
expect "accept/reject overlap"   2 "both accepting and rejecting" "$BIN" tests/bad/accept_reject_overlap.tm --check
expect "--check needs a file"    2 "needs a machine file" "$BIN" --check

echo "Interactive mode"
expect "typed-in session"        0 "RESULT: ACCEPTED"  sh -c "'$BIN' < examples/binary_increment.txt"
# type a machine, save it, then run the saved file
printf '2\n0 1 a b R\n1 2 b b S\n0\n2\n\n\n%s/saved.tm\nab\nn\nn\n' "$TMP" | "$BIN" > "$TMP/session.out" 2>&1
if grep -q "Saved" "$TMP/session.out" && [ -f "$TMP/saved.tm" ]; then ok "save machine to file"; else bad "save machine to file"; fi
expect "saved file runs"         0 "ACCEPTED"          "$BIN" "$TMP/saved.tm" -i ab -q
printf '1\n0 1 a b R\n0\n1\n\n\n%s/saved.tm\nn\nab\nn\nn\n' "$TMP" | "$BIN" > "$TMP/session2.out" 2>&1
if grep -q "already exists" "$TMP/session2.out"; then ok "asks before overwriting"; else bad "asks before overwriting"; fi
printf 'abc\n0\n1\n' | "$BIN" > "$TMP/bad.out" 2>&1; code=$?
if [ $code -eq 1 ] && grep -q "Input ended unexpectedly" "$TMP/bad.out"; then ok "EOF does not hang"; else bad "EOF does not hang (exit $code)"; fi

echo "Update checker (local file:// URLs)"
V=$(tr -d ' \r\n' < VERSION)
echo "99.0.0" > "$TMP/v_new"; echo "$V" > "$TMP/v_same"; echo "<html>not a version</html>" > "$TMP/v_junk"
expect "newer version found"     4 "99.0.0"            env TIMSIM_VERSION_URL="file://$TMP/v_new"  "$BIN" --check-update
expect "already up to date"      0 "up to date"        env TIMSIM_VERSION_URL="file://$TMP/v_same" "$BIN" --check-update
expect "junk is not trusted"     1 "Could not check"   env TIMSIM_VERSION_URL="file://$TMP/v_junk" "$BIN" --check-update
expect "unreachable version file" 1 "Could not check"  env TIMSIM_VERSION_URL="file://$TMP/nothing" "$BIN" --check-update
expect "unsafe URL refused"      1 "Could not check"   env TIMSIM_VERSION_URL="file:///x;touch $TMP/pwned" "$BIN" --check-update
if [ ! -e "$TMP/pwned" ]; then ok "no shell injection via URL"; else bad "shell injection via URL"; fi
printf '#!/bin/sh\necho "FAKE-INSTALLER prefix=$PREFIX"\n' > "$TMP/fake_install.sh"
expect "--update runs installer" 0 "FAKE-INSTALLER prefix=" env TIMSIM_VERSION_URL="file://$TMP/v_new" TIMSIM_INSTALL_URL="file://$TMP/fake_install.sh" "$BIN" --update
expect "--update failed download" 1 "Update failed"     env TIMSIM_VERSION_URL="file://$TMP/v_new" TIMSIM_INSTALL_URL="file://$TMP/missing.sh" "$BIN" --update
expect "--update when current"   0 "up to date"        env TIMSIM_VERSION_URL="file://$TMP/v_same" "$BIN" --update

# automatic check needs a real terminal, so run inside a pseudo terminal with script(1)
if script -qec true /dev/null >/dev/null 2>&1; then
    unset TIMSIM_NO_UPDATE_CHECK
    export XDG_CACHE_HOME="$TMP/cache" TIMSIM_VERSION_URL="file://$TMP/v_new"
    out=$(printf 's\n1011\nn\nn\n' | script -qec "$BIN examples/binary_increment.tm" /dev/null 2>&1)
    if printf '%s' "$out" | grep -q "new version of TimSim is available"; then ok "auto check offers update"; else bad "auto check offers update"; fi
    if grep -q "99.0.0" "$TMP/cache/timsim/update_check" 2>/dev/null; then ok "skip choice remembered"; else bad "skip choice remembered"; fi
    out=$(printf '1011\nn\nn\n' | script -qec "$BIN examples/binary_increment.tm" /dev/null 2>&1)
    if printf '%s' "$out" | grep -q "new version of TimSim"; then bad "no second prompt within a day"; else ok "no second prompt within a day"; fi
    echo "1 -" > "$TMP/cache/timsim/update_check"   # pretend the last check was long ago
    out=$(printf 'n\n1011\nn\nn\n' | script -qec "$BIN examples/binary_increment.tm" /dev/null 2>&1)
    if printf '%s' "$out" | grep -q "new version of TimSim is available"; then ok "re-checks after a day"; else bad "re-checks after a day"; fi
    export TIMSIM_NO_UPDATE_CHECK=1
else
    echo "  SKIP  automatic check tests (script(1) with -c not available)"
fi

echo
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
