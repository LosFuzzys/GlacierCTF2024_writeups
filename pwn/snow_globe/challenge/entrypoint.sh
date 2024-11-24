#!/bin/sh

# Set up the directories in the tempdir
mkdir -m 777 /tmp/snow_globe
mkdir -m 777 /tmp/snow_globe/uploads
mkdir -m 777 /tmp/snow_globe/intermediate_results
mkdir -m 770 /tmp/snow_globe/results

. /.venv/bin/activate 2>&1 >/dev/null
export PATH="/.venv/bin:$PATH"
/usr/bin/stdbuf -i0 -o0 -e0 /app/challenge
