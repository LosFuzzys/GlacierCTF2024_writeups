#!/bin/sh
export PATH="${PATH}:/usr/libexec/gcc/x86_64-alpine-linux-musl/13.2.1/" # why :cry:
/usr/bin/stdbuf -i0 -o0 -e0 /app/challenge
