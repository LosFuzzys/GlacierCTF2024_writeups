#!/bin/bash

_term() {
  echo "Caught SIGTERM signal!"
  [[ $redis_server ]] && kill -TERM "$redis_server"
  [[ $app_service ]] && kill -TERM "$app_service"
  [[ $bot_service ]] && kill -TERM "$bot_service"
}

_int() {
  echo "Caught SIGINT signal!"
  [[ $redis_server ]] && kill -TERM "$redis_server"
  [[ $app_service ]] && kill -TERM "$app_service"
  [[ $bot_service ]] && kill -INT "$bot_service"
}

trap _term SIGTERM
trap _int SIGINT

. /.venv/bin/activate 2>&1 >/dev/null
export PATH="/.venv/bin:$PATH"

echo "[+] Booting up challenge"
cd /app/src
redis-server &
redis_server=$!
python app.py 2>&1 &
app_service=$!
rq worker --with-scheduler --url redis://localhost:6379 2>&1 &
bot_serivce=$!

wait -n
exit $?

