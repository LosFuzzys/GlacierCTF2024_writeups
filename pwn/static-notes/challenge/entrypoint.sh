#!/bin/bash

MAX_SIZE=0x5000

echo "[+] Example: tar cz notes.json | base64 && echo "@""
echo "[>] --- BASE64 INPUT START ---"
read -r -d @ FILE
echo "[>] --- BASE64 INPUT END ---"

cp -r /app /tmp/app
echo "${FILE}" | base64 -d 2>/dev/null | tar -xzO >/tmp/notes.json 2>/dev/null
if [ $? -ne 0 ] || [ ! -f /tmp/notes.json ]; then
  echo "[-] Failed to parse your notes, using ours"
else
  mv /tmp/notes.json /tmp/app/src/notes.json
fi

if (($(stat -c %s /tmp/notes.json) > $MAX_SIZE)); then
  echo "[-] File too large"
  exit 1
fi

cd /tmp/app
zig build --global-cache-dir /tmp/app/.zig-cache
if [ $? -ne 0 ]; then
  echo "[-] Build failed"
  exit 1
fi
echo "[+] Build finished"

echo "[?] Do you want a copy of the finished notes app 📝"
read -p "[?] (y/n) " -r
if [[ $REPLY =~ ^[Yy]$ ]]; then
  tar cz /tmp/app/zig-out/bin/challenge 2>/dev/null | base64
fi

echo "[>] Launching..."
exec /tmp/app/zig-out/bin/challenge
exit 0
