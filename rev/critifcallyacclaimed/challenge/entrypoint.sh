#!/bin/sh

export RUST_LOG=info
cd /app && /usr/bin/stdbuf -i0 -o0 -e0 ./challenge
