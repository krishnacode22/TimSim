#!/bin/sh
# TimSim test suite.  Usage: sh tests/run_tests.sh ./timsim   (or: make test)
# No internet is needed.
BIN=$(cd "$(dirname "$1")" && pwd)/$(basename "${1:-timsim}")
cd "$(dirname "$0")/.." || exit 1
ROOT=$(pwd)
TMP=$(mktemp -d)
# A Windows timsim.exe can not open MSYS paths such as /tmp/..., so give it a native path.
if command -v cygpath >/dev/null 2>&1; then TMP=$(cygpath -m "$TMP"); fi
trap 'rm -rf "$TMP"' EXIT
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

echo "More behaviour"
printf 'start 0\naccept 9\nlimit 5\n0 0 _ _ R\n0 0 a a R\n' > "$TMP/limit.tm"
expect "limit directive in file" 3 "STEP_LIMIT"      "$BIN" "$TMP/limit.tm" -i "" -q
printf 'start 0\naccept 1\n0 1 a b r\n' > "$TMP/lower.tm"
expect "lowercase move letter"   0 "ACCEPTED"        "$BIN" "$TMP/lower.tm" -i a -q
printf 'start 0\naccept 1\n0 0 a b S\n' > "$TMP/notrans.tm"
expect "no transition rejects"   1 "REJECTED"        "$BIN" "$TMP/notrans.tm" -i aa -q
printf 'start 0\naccept 9\n0 0 _ _ R\n0 0 a a R\n' > "$TMP/right.tm"
expect "tape edge is no decision" 3 "TAPE_LIMIT"     "$BIN" "$TMP/right.tm" -i "" -l 20000 -q
expect "tape edge message"       3 "TAPE LIMIT REACHED" "$BIN" "$TMP/right.tm" -i "" -l 20000
expect "--run FILE"              0 "1100"            "$BIN" --run examples/binary_increment.tm -i 1011 -q
expect "--check reports warnings" 0 "never used as a next state" "$BIN" examples/loops_forever.tm --check
expect "--demo quiet"            0 "1100"            "$BIN" --demo -q
expect "error summary"           2 "5 error(s) found"      "$BIN" tests/bad/many_errors.tm --check
expect "bad --limit value"       2 "step limit must be"    "$BIN" examples/palindrome.tm -i 1 -l abc
expect "demo plus file"          2 "either --demo or"      "$BIN" --demo examples/palindrome.tm
expect "reject state message"    1 "reached rejecting state 3" "$BIN" examples/even_zeros.tm -i 10
printf 'start 0   # begin here\naccept 1  # done\n\n# a whole-line comment\n0 1 a b R   # rule\n' > "$TMP/comments.tm"
expect "comments are ignored"    0 "ACCEPTED"        "$BIN" "$TMP/comments.tm" -i a -q
LONG200=$(printf '%0200d' 0 | tr 0 1)
LONG201=$(printf '%0201d' 0 | tr 0 1)
expect "input of 200 chars is ok" 0 "ACCEPTED"       "$BIN" examples/even_zeros.tm -i "$LONG200" -q
expect "input of 201 chars fails" 2 "too long"       "$BIN" examples/even_zeros.tm -i "$LONG201" -q
i=0; { echo "start 0"; echo "accept 31"; while [ $i -le 30 ]; do echo "$i $((i + 1)) a a R"; i=$((i + 1)); done; } > "$TMP/many.tm"
expect "31 transitions refused"  2 "too many transitions" "$BIN" "$TMP/many.tm" --check
for f in examples/*.tm; do expect "example is valid: $f" 0 "valid deterministic" "$BIN" "$f" --check; done
if printf '%s' "$(tr -d ' \r\n' < VERSION)" | grep -Ec '^[0-9]+\.[0-9]+\.[0-9]+$' >/dev/null; then ok "VERSION is X.Y.Z"; else bad "VERSION is X.Y.Z"; fi

echo "Colour"
if "$BIN" examples/binary_increment.tm -i 1011 | grep -q "$(printf '\033')"; then bad "piped output stays plain"; else ok "piped output stays plain"; fi
if "$BIN" examples/binary_increment.tm -i 1011 --color=always | grep -q "$(printf '\033')"; then ok "--color=always emits codes"; else bad "--color=always emits codes"; fi
if NO_COLOR=1 "$BIN" examples/binary_increment.tm -i 1011 --color=always | grep -q "$(printf '\033')"; then ok "--color=always ignores NO_COLOR"; else bad "--color=always ignores NO_COLOR"; fi
if "$BIN" examples/binary_increment.tm -i 1011 -q --color=always | grep -q "$(printf '\033')"; then bad "-q stays plain"; else ok "-q stays plain"; fi
if "$BIN" examples/binary_increment.tm -i 1011 --color=always | grep -q "RESULT: ACCEPTED"; then ok "coloured result keeps the words"; else bad "coloured result keeps the words"; fi
if "$BIN" examples/binary_increment.tm -i 1011 --color=always | grep -q "Final tape  : 1100"; then ok "coloured output keeps the tape line"; else bad "coloured output keeps the tape line"; fi
expect "bad --color value"       2 "--color needs"          "$BIN" --color=sometimes

echo "Step mode"
STEPS="printf '\n\n\n\n\n\n\n\n\n\n'"
expect "step mode pauses"        0 "Press Enter for the next step" sh -c "$STEPS | '$BIN' examples/binary_increment.tm -i 1011 --step"
expect "step mode implies trace" 0 "Execution trace" sh -c "$STEPS | '$BIN' examples/binary_increment.tm -i 1011 --step"
expect "step mode finishes"      0 "RESULT: ACCEPTED" sh -c "$STEPS | '$BIN' examples/binary_increment.tm -i 1011 --step"
if "$BIN" examples/binary_increment.tm -i 1011 -t | grep -q "Press Enter"; then bad "trace without --step does not pause"; else ok "trace without --step does not pause"; fi

echo "Animation"
expect "animate falls back when piped" 0 "RESULT: ACCEPTED" "$BIN" examples/binary_increment.tm -i 1011 --animate 20
if "$BIN" examples/binary_increment.tm -i 1011 --animate 20 | grep -q "$(printf '\r')"; then bad "animate is silent when piped"; else ok "animate is silent when piped"; fi
expect "bad animate delay"      2 "delay must be"        "$BIN" examples/binary_increment.tm -i 1011 --animate 5
expect "animate needs a value"  2 "needs a value"        "$BIN" examples/binary_increment.tm -i 1011 --animate
expect "step and animate clash" 2 "can not be used together" "$BIN" examples/binary_increment.tm -i 1011 --step --animate 20

echo "Interactive mode"
expect "typed-in session"        0 "RESULT: ACCEPTED"  sh -c "cd '$TMP' && '$BIN' < '$ROOT/examples/binary_increment.txt'"
expect "transcript accept"       0 "RESULT: ACCEPTED"  sh -c "cd '$TMP' && '$BIN' < '$ROOT/examples/even_zeros_accept.txt'"
expect "transcript reject"       0 "RESULT: REJECTED"  sh -c "cd '$TMP' && '$BIN' < '$ROOT/examples/even_zeros_reject.txt'"
expect "transcript step limit"   0 "STEP LIMIT REACHED" sh -c "cd '$TMP' && '$BIN' < '$ROOT/examples/step_limit.txt'"
# type a machine, save it, then run the saved file
printf '2\n0 1 a b R\n1 2 b b S\n0\n2\n\n\n%s/saved.tm\nab\nn\nn\n' "$TMP" | "$BIN" > "$TMP/session.out" 2>&1
if grep -q "Saved" "$TMP/session.out" && [ -f "$TMP/saved.tm" ]; then ok "save machine to file"; else bad "save machine to file"; fi
expect "saved file runs"         0 "ACCEPTED"          "$BIN" "$TMP/saved.tm" -i ab -q
printf '1\n0 1 a b R\n0\n1\n\n\n%s/saved.tm\nn\n\nab\nn\nn\n' "$TMP" | "$BIN" > "$TMP/session2.out" 2>&1
if grep -q "already exists" "$TMP/session2.out"; then ok "asks before overwriting"; else bad "asks before overwriting"; fi
printf 'abc\n0\n1\n' | "$BIN" > "$TMP/bad.out" 2>&1; code=$?
if [ $code -eq 1 ] && grep -q "Input ended unexpectedly" "$TMP/bad.out"; then ok "EOF does not hang"; else bad "EOF does not hang (exit $code)"; fi

echo "Install layout"
# make install / uninstall into a temporary folder (only when testing the ./timsim built by make)
if [ "$BIN" = "$ROOT/timsim" ] && [ -f Makefile ] && command -v make >/dev/null 2>&1; then
    if make install PREFIX="$TMP/prefix" >/dev/null 2>&1 \
        && [ -x "$TMP/prefix/bin/timsim" ] && [ -f "$TMP/prefix/share/timsim/examples/palindrome.tm" ]; then
        ok "make install: program and examples"
    else
        bad "make install: program and examples"
    fi
    expect "installed example runs"  0 "ACCEPTED" "$TMP/prefix/bin/timsim" "$TMP/prefix/share/timsim/examples/palindrome.tm" -i 0110 -q
    make uninstall PREFIX="$TMP/prefix" >/dev/null 2>&1
    if [ ! -e "$TMP/prefix/bin/timsim" ] && [ ! -e "$TMP/prefix/share/timsim" ]; then ok "make uninstall removes everything"; else bad "make uninstall removes everything"; fi
else
    echo "  SKIP  install tests (only run for the ./timsim built by make)"
fi

echo "Terminal behaviour"
# animation only redraws on a real terminal, so run inside a pseudo terminal with script(1)
if script -qec true /dev/null >/dev/null 2>&1; then
    out=$(printf '' | script -qec "$BIN examples/binary_increment.tm -i 1011 --animate 20" /dev/null 2>&1)
    if printf '%s' "$out" | grep -q "RESULT: ACCEPTED" && printf '%s' "$out" | grep -q "$(printf '\r')"; then ok "animate redraws in a tty"; else bad "animate redraws in a tty"; fi
else
    echo "  SKIP  terminal tests (script(1) with -c not available)"
fi

echo
echo "$pass passed, $fail failed"
[ "$fail" -eq 0 ]
