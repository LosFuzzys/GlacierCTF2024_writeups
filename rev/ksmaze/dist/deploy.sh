#!/bin/sh

check() {
  echo -e "\e[1;34m[+] Verifying Challenge Integrity\e[0m"
  sha256sum -c sha256sum
}

build_container() {
  echo -e "\e[1;34m[+] Building Challenge Docker Container\e[0m"
  docker build -t localhost/chall-ksmaze --platform linux/amd64 . 
}

run_container() {
  echo -e "\e[1;34m[+] Running Challenge Docker Container on 0.0.0.0:1337\e[0m"
  docker run --name chall-ksmaze --rm -p 0.0.0.0:1337:1337 -t -i -e HOST=0.0.0.0 -e PORT=1337 -e TIMEOUT=500 -e PUBPORTSTART=20000 -e DOMAIN=78.47.52.31 -e PUBPORTEND=21000 -p 0.0.0.0:20000-21000:20000-21000 --user 1337:1337 --read-only --tmpfs=/tmp --privileged --platform linux/amd64 localhost/chall-ksmaze
}

kill_container() {
	docker ps --filter "name=chall-ksmaze" --format "{{.ID}}" \
		| tr '\n' ' ' \
		| xargs docker stop -t 0 \
		|| true
}

case "${1}" in
  "check")
    check
    ;;
  "build")
    build_container
    ;;
  "run")
    run_container
    ;;
  "kill")
    kill_container
    ;;
  *)
    check
    build_container
    run_container
    ;;
esac
