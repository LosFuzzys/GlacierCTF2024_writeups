#!/usr/bin/env bash

while true;
do
	for f in /tmp/tmp.*--ssd-qemu; do
		
		if [ "${f}" == "/tmp/tmp.*--ssd-qemu" ]
		then
			echo "[+] Nothing to clean"
			continue
		fi

		# We can't use the PIDfile as PIDs can be reused by a new process
		out=`ps aux | grep ${f} | grep -v grep`
		if [ -z "${out}" ]
		then
			echo "[+] Cleaning up ${f} as its dangling"
			rm -rf ${f}
		else
			echo "[+] Not cleaning up ${f} as its being used"
		fi
	done

	sleep 10
done
