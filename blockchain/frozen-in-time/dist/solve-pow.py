#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import hashlib
import random


def main():
    print("Finging ticket .... ", end="", flush=True)

    x = 100000000 + random.randint(0, 200000000000)

    for i in range(x, x + 20000000000):
        m = hashlib.sha256()
        ticket = str(i)
        m.update(ticket.encode('ascii'))
        digest1 = m.digest()
        m = hashlib.sha256()
        m.update(digest1 + ticket.encode('ascii'))
        if m.hexdigest().startswith('0000000'):
            print("Finished!")
            print(f"Ticket found: {i}")
            break


if __name__ == "__main__":
    raise SystemExit(main())
