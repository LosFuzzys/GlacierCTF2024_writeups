#!/bin/sh
. /.venv/bin/activate 2>&1 >/dev/null
export PATH="/.venv/bin:$PATH"
/app/challenge
