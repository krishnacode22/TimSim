# Adds 1 to a binary number (the same machine as --demo).
# State 0 runs right to the end of the number, state 1 walks back turning
# 1s into 0s until it can write a 1, state 2 accepts.
start 0
accept 2
limit 1000

0 0 0 0 R
0 0 1 1 R
0 1 _ _ L

1 1 1 0 L
1 2 0 1 S
1 2 _ 1 S
