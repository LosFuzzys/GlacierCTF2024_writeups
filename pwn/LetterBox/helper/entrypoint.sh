#!/bin/sh

echo "[+] Running in context $(id)"
echo "[+] Loading challenge container image"
podman load -i /app/challenge.tar

echo "[+] Running cleaner in the background"
/app/cleaner.sh &

echo "[+] Running caddy proxy server"
caddy run --config /app/Caddyfile &

touch /home/podman/port.lock
echo "0" > /home/podman/port
socat TCP-LISTEN:1337,fork,nodelay,reuseaddr,pktinfo EXEC:"/usr/bin/timeout -k 5 ${TIMEOUT} /app/run.sh"
