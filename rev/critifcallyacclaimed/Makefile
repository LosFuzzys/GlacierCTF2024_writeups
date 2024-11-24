.PHONY: all run build solve kill dist distrun clean deploy

# Targets
all: build run
run: crun
build: cbuild
solve: sbuild srun
kill: ckill skill

######################
# CTFd Configuration #
######################

export FULLNAME = critifcallyacclaimed
export AUTHOR   = riggle
export CATEGORY = rev
export DESC     = TODO
export FLAG     = $(shell cat challenge/flag.txt)
export TAG      = CTFd tag

############################
# Deployment Configuration #
############################

export REGISTRY = localhost
export HOST     = 127.0.0.1
# Port for private deployments
export PORT     = 1337
export TIMEOUT  = 0
# Port for public deployments
export PUBPORT  = 10000

# Slugified challenge name. Default to directory name
MKPATH:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
UNCLEAN_NAME=$(notdir ${MKPATH})
export NAME=$(shell echo ${UNCLEAN_NAME} | tr "[:upper:]" "[:lower:]")

########################
# Challenge Dockerfile #
########################

CACHEBUST=$(shell date +%s)

# Docker Run Args
export RARGS ?= -p ${HOST}:${PORT}:1337 -t -i -e HOST=${HOST} -e PORT=${PORT} \
	-e TIMEOUT=${TIMEOUT} --user 1337:1337 --read-only --security-opt=no-new-privileges:true \
	--platform linux/amd64
# Docker Build Args
export BARGS ?= --platform linux/amd64 --build-arg CACHEBUST=${CACHEBUST}

crun:
	@echo -e "\e[1;34m[+] Running Challenge Container on ${HOST}:${PORT}\e[0m"
	docker run --rm --name ${NAME} ${RARGS} ${REGISTRY}/${NAME}

cbuild:
	@echo -e "\e[1;34m[+] Building Challenge Container\e[0m"
	docker build -t ${REGISTRY}/${NAME}-build --target builder ${BARGS} .
	docker run  -v ${MKPATH}/dist:/dist:z ${REGISTRY}/${NAME}-build
	cp ${MKPATH}/dist/challenge ${MKPATH}/solution/
	docker build -t ${REGISTRY}/${NAME} ${BARGS} .

ckill:
	@echo -e "\e[1;34m[+] Killing Challenge Container\e[0m"
	docker ps --filter "name=${NAME}" --format "{{.ID}}" \
		| tr '\n' ' ' \
		| xargs docker stop -t 0 \
		|| true

##########################
# Challenge Distribution #
##########################

HSH := $(shell md5sum 2>/dev/null ${NAME}.tar.gz | cut -d' ' -f1)

# If tar complains about --transform, ensure you have GNU tar
dist: build
	@echo -e "\e[1;34m[+] Generating Distributed ${NAME}.tar.gz\e[0m"
	envsubst < ./dist/deploy.sh.template | \
		tee ./dist/deploy.sh >/dev/null
	chmod +x ./dist/deploy.sh
	cd dist && sha256sum * | sed -e '/sha256sum/d' -e '/deploy.sh.template/d' \
		-e '/.gitignore/d' | tee sha256sum
	tar -h --exclude 'deploy.sh.template' --exclude '.gitignore' \
		--transform s@^dist@${NAME}@ -czvf \
		${NAME}.tar.gz dist/
	@echo -e "\e[1;34m[+] Generated Distributed ${NAME}.tar.gz\e[0m"
	@echo -n "    md5sum: (old) ${HSH} -> (new) "
	@md5sum ${NAME}.tar.gz | cut -d' ' -f1

# Deploy the dist file for testing (as a player will do)
distrun:
	@echo -e "\e[1;34m[+] Deploying Distributed ${NAME}.tar.gz (${HSH})\e[0m"
	cd $(shell mktemp -d --suffix _distrun) && \
	cp ${MKPATH}/${NAME}.tar.gz . && \
	tar xf ${NAME}.tar.gz && \
	cd ${NAME} && \
	./deploy.sh

#########################
# Challenge Solvescript #
#########################

# Docker Run Args
SRARGS=-e HOST=${HOST} -e PORT=${PORT} -e TIMEOUT=500 \
			 --hostname localhost --net=host \
			 -v ${MKPATH}/challenge/flag.txt:/app/flag.txt:z,ro \
			 -v ${MKPATH}/challenge/flag.txt:/flag.txt:z,ro \
			 -v ${MKPATH}/dist/flag.txt:/app/flag-fake.txt:z,ro \
			 -v ${MKPATH}/dist/flag.txt:/flag-fake.txt:z,ro \
			 --platform linux/amd64
# Docker Build Args
SBARGS=--platform linux/amd64

sbuild:
	@echo -e "\e[1;34m[+] Building Solvescript Container\e[0m"
	cd solution/ ; \
	docker build -t ${REGISTRY}/${NAME}-solvescript ${SBARGS} .

srun:
	@echo -e "\e[1;34m[+] Running Solvescript Container against" \
		"${HOST}:${PORT}\e[0m"
	cd solution/ ; \
	docker run --rm ${SRARGS} \
		--name ${NAME}-solvescript-oneshot ${REGISTRY}/${NAME}-solvescript

TIMES=10
srun-sequential:
	@echo -e "\e[1;34m[+] Running Sequential Container against" \
		"${HOST}:${PORT}\e[0m"
	cd solution/ ; \
	parallel --jobs 1 docker run --rm ${SRARGS} \
		--name ${NAME}-solvescript-sequential-{} ${REGISTRY}/${NAME}-solvescript \
		::: {1..${TIMES}}
	
JOBS=2
srun-parallel:
	@echo -e "\e[1;34m[+] Running Parallel Container against ${HOST}:${PORT}\e[0m"
	cd solution/ ; \
	parallel --jobs ${JOBS} docker run --rm ${SRARGS} \
		--name ${NAME}-solvescript-parallel-{} ${REGISTRY}/${NAME}-solvescript \
		::: {1..${TIMES}}

skill:
	@echo -e "\e[1;34m[+] Killing Solution Container/s\e[0m"
	docker ps --filter "name=${NAME}-solvescript*" --format "{{.ID}}" \
		| tr '\n' ' ' \
		| xargs docker stop -t 0 \
		|| true

########################
# Challenge Deployment #
########################
deploy: deploy-registry

deploy-yml:
	@echo -e "\e[1;34m[+] Generating CTFd challenge yml\e[0m"
	envsubst < ${MKPATH}/deployment/ctfd-entry.yml.template | \
		tee ${MKPATH}/deployment/ctfd-${NAME}.yml >/dev/null

# Deployments is prepared for quadlets (containers + systemd services)
# https://www.redhat.com/sysadmin/quadlet-podman
# - Assuming running user is the one that is operating this Makefile
# - Remember to enable lingering for the user: loginctl enable-linger $USER
# - Configure this user with the minimum privileges (not in sudoers)
#
# The service will automatically be enabled after reboot due to the [Install]
# target in the quadlet/systemd service. So, reboot the machine or start it
# once, and then it will always be running unless stopped.
deploy-quadlet:
	@echo -e "\e[1;34m[+] Deploying Container on 0.0.0.0:${PUBPORT}\e[0m"
	mkdir -p ~/.config/containers/systemd/
	envsubst < ${MKPATH}/deployment/systemd-service.container.template | \
		tee ~/.config/containers/systemd/chall-${NAME}.container >/dev/null
	systemctl --user daemon-reload
	@echo -e "\e[1;34m[+] Start the systemd service:\e[0m"
	systemctl --user status chall-${NAME} || true
	@echo -e "\e[1;34m[+] Remember to enable (once) lingering for the user\e[0m"
	@echo    "    loginctl enable-linger $$USER"

deploy-registry: 
	@echo -e "\e[1;34m[+] Deploying Container/s\e[0m"
	docker push ${REGISTRY}/${NAME}

###########
# Generic #
###########

clean:
	${RM} ${NAME}.tar.gz dist/challenge dist/deploy.sh dist/sha256sum \
		solution/challenge ctfd-${NAME}.yml
	make -C challenge clean

# vim:tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab colorcolumn=81
