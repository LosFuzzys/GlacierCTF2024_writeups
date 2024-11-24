#!/usr/bin/env sagemath
# -*- coding: utf-8 -*-
# Respect the shebang and mark file as executable

import os

from Crypto.Util.number import getPrime
from Crypto.Util.number import isPrime
from Crypto.Util.number import bytes_to_long

def generate_primes():
    while True:
        p = getPrime(512),
        p = 9352496155192295944243473644483853835662636576410969996619180877861158926367873785037099054018741236476166923118647057249968914650337399039210616026612969
        q = (2*p) + 1
        r = (2*q) + 1
        s = (2*r) + 1

        if isPrime(q) and isPrime(r) and isPrime(s):
            break
    
    return (p, q, r, s)


def main() -> int:
    (p, q, r, s) = generate_primes()
    N = p * q * r * s
    e = 0x10001

    with open("flag.txt", "r") as flag_file:
        flag = flag_file.read().strip()

    CT = pow(bytes_to_long(flag.encode()), e, N)

    print(f"{N = }")
    print(f"{CT = }")


    return 0

if __name__ == '__main__':
    raise SystemExit(main())

# vim:filetype=python
