#!/usr/bin/env bash

while true;
do
	for f in /tmp/tmp.*--glacier-nginx; do
		
		if [ "${f}" == "/tmp/tmp.*--glacier-nginx" ]
		then
			echo "[+] Nothing to clean"
			continue
		fi

		# We can't use the PIDfile as PIDs can be reused by a new process
		out=`podman ps --format "{{.CIDFile}}" | grep ${f} | grep -v grep`
		if [ -z "${out}" ]
		then
			DOMAIN=$(cat ${f}/domain)
			echo "[+] Removing subdomain $DOMAIN from caddy"
			curl -X DELETE "http://localhost:2019/id/$DOMAIN" 
			echo "[+] Cleaning up ${f} as its dangling"
			rm -rf ${f}
		else
			echo "[+] Not cleaning up ${f} as its being used"
		fi
	done

	sleep ${TIMEOUT}
done
