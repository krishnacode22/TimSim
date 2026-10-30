# Accepts binary palindromes (the empty string counts as a palindrome).
# A matching pair is marked x (for 0) or y (for 1) on both sides.
# State 0 marks the leftmost unmarked symbol, states 1 and 5 run right,
# states 2 and 6 compare the rightmost unmarked symbol, state 3 walks back.
start 0
accept 4
reject 7
limit 1000

0 1 0 x R
0 5 1 y R
0 0 x x R
0 0 y y R
0 4 _ _ S

1 1 0 0 R
1 1 1 1 R
1 2 x x L
1 2 y y L
1 2 _ _ L

5 5 0 0 R
5 5 1 1 R
5 6 x x L
5 6 y y L
5 6 _ _ L

2 3 0 x L
2 4 x x S
2 4 y y S
2 7 1 y L

6 3 1 y L
6 4 x x S
6 4 y y S
6 7 0 x L

3 3 0 0 L
3 3 1 1 L
3 0 x x R
3 0 y y R
3 0 _ _ R
