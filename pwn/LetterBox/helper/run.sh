#!/usr/bin/env bash

# Generate temporary directory per connection
DIR=$(mktemp -d --suffix=--glacier-nginx)
cd ${DIR}

echo -e "Press [ENTER] to start a new instance"
read -n1

SUBDOMAIN=$(echo $RANDOM | sha1sum | head -c 32)
echo "${SUBDOMAIN}" > domain

exec 100>/home/podman/port.lock || (echo "Could not spawn instance, please contact an administrator!"; exit -1)
flock -x -w 10 100 
CURRENT_PORT=`cat /home/podman/port`
expr \( $CURRENT_PORT + 2 \) % \( $PUBPORTEND - $PUBPORTSTART \) > /home/podman/port
flock -u 100

PORT=`expr $CURRENT_PORT + $PUBPORTSTART`
PORT_PWN=`expr $CURRENT_PORT + 1 + $PUBPORTSTART`


# PORT=`shuf -i ${PUBPORTSTART}-${PUBPORTEND} -n1`
echo "${PORT}" > port


HOSTNAME=$(echo $RANDOM | md5sum | head -12)
podman run -d --uts=private --hostname "${HOSTNAME}" --timeout ${TIMEOUT} --cidfile=${DIR}/cid -p "127.0.0.1:${PORT}:1337" -p "${PORT_PWN}:8081" --network=pasta ${REGISTRY}/${NAME}-challenge 2>/dev/null 1>/dev/null

FQDN=${SUBDOMAIN}.${DOMAIN}

PAYLOAD="{\"@id\": \"${SUBDOMAIN}\", \"handle\":[{\"handler\":\"subroute\",\"routes\":[{\"handle\":[{\"handler\":\"reverse_proxy\",\"upstreams\":[{\"dial\":\"127.0.0.1:${PORT}\"}]}]}]}],\"match\":[{\"host\":[\"${FQDN}\"]}],\"terminal\":true}"
curl -X PUT -H "Content-Type: application/json" -d "${PAYLOAD}" "http://localhost:2019/config/apps/http/servers/srv0/routes/0" 

if [[ ! -z "${DOMAIN_PORT}" ]]; then
	FQDN=${FQDN}:${DOMAIN_PORT}
fi

echo -e "\e[1;34m[+] ${DOMAIN_PROT}://${FQDN}\e[0m"
echo -e "\e[1;34m[+] nc ${HOST} ${PORT_PWN}\e[0m"
echo -e "\e[1;34m[+] Wait some time until the challenge is fully booted up\e[0m"

echo -e "\e[1;34m[+] You have ${TIMEOUT} seconds to solve it. Avoid timeouts by running it locally.\e[0m"

echo ""
echo -e "Press [ENTER] to stop the instance"
read -n1

CID=`cat cid`
podman stop -t 0 ${CID} 2>&1 >/dev/null

echo -e "Instance stopped"
