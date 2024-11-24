# Writeup

This challenge uses shamier secret sharing to "share" a binary.
Because the sharing threshold (2) is lower then the number of shares (3) that are available, it is possible to reconstruct the original file (in this case a elf binary).

To make it a bit harder, the binary to reverse engineer is written in go, but after spotting that shamier secret sharing is being used, it should be possible to solve the challenge with ease.