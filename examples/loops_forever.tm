# Runs to the right forever, so only the step limit stops it.
# The blank transition is needed for an empty input string.
# The accepting state 1 is never reached during the run.
start 0
accept 1
limit 1000

0 0 _ _ R
0 0 0 0 R
0 0 1 1 R
