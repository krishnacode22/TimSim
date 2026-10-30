# Accepts binary strings with an even number of 0s.
# State 0 means "even so far", state 1 means "odd so far".
start 0
accept 2
reject 3
limit 1000

0 1 0 0 R
0 0 1 1 R
0 2 _ _ S

1 0 0 0 R
1 1 1 1 R
1 3 _ _ S
