#!/bin/sh

while true; do
  echo "[+] Running cronjob" 
  /usr/local/bin/php /var/www/cron.php 2>&1 &
  sleep 20 
done;
