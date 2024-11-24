#!/bin/bash

/usr/local/bin/php /var/www/setup.php

chmod -R 777 /tmp/

_term() {
  echo "Caught SIGTERM signal!"
  [[ $apache_service ]] && kill -TERM "$apache_service"
  [[ $cron_service ]] && kill -TERM "$cron_service"
}

_int() {
  echo "Caught SIGINT signal!"
  [[ $apache_service ]] && kill -INT "$apache_service"
  [[ $cron_service ]] && kill -INT "$cron_service"
}

trap _term SIGTERM
trap _int SIGINT

apache2-foreground 2>&1 &
apache_service=$!
/service/cron.sh 2>&1 &
cron_service=$!

wait -n
exit $?
