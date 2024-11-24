.PHONY: all run build solve kill dist distrun clean deploy

# Targets
all: build run
run: crun
build: cbuild
solve: sbuild srun
kill: ckill skill

solve_remote: HOST=challs.glacierctf.com 
solve_remote: PORT=13375
solve_remote:  sbuild srun

######################
# CTFd Configuration #
######################

export FULLNAME = Snow Globe 🌍❄
export AUTHOR   = huksys
export CATEGORY = pwn
export DESC     = $(shell cat DESCRIPTION)
export FLAG     = $(shell cat challenge/flag.txt)
export TAG      = pwn

############################
# Deployment Configuration #
############################

export REGISTRY = localhost
export HOST     = 127.0.0.1
# Port for private deployments
export PORT     = 1337
export TIMEOUT  = 600
# Port for public deployments
export PUBPORTSTART  = 11000
export PUBPORTEND    = 15000
# Domain/subdomain of instanced challenges <RANDOM>.${DOMAIN}
export DOMAIN        = localhost
export DOMAIN_PORT	 = 8080

# Slugified challenge name. Default to directory name
MKPATH:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
UNCLEAN_NAME=$(notdir ${MKPATH})
export NAME=$(shell echo ${UNCLEAN_NAME} | tr "[:upper:]" "[:lower:]")

########################
# Challenge Dockerfile #
########################

CACHEBUST=$(shell date +%s)

# Docker Run Args
export RARGS ?= \
	-p ${HOST}:${PORT}:${PORT} -p ${HOST}:8080:8080 -it -e HOST=${HOST} -e PORT=${PORT} \
	-e PUBPORTSTART=${PUBPORTSTART} -e PUBPORTEND=${PUBPORTEND} -e NAME=${NAME} \
	-e TIMEOUT=${TIMEOUT} -e DOMAIN=${DOMAIN} -e DOMAIN_PORT=${DOMAIN_PORT} \
	-e DOMAIN_PROT=http -e REGISTRY=${REGISTRY} --privileged --platform linux/amd64
# Docker Build Args
export BARGS ?= --platform linux/amd64 --build-arg CACHEBUST=${CACHEBUST}

crun:
	@echo -e "\e[1;34m[+] Running Challenge Container on ${HOST}:${PORT}\e[0m"
	docker run --rm --name ${NAME} ${RARGS} ${REGISTRY}/${NAME}

cbuild:
	@echo -e "\e[1;34m[+] Building Challenge Container\e[0m"
	docker build -t ${REGISTRY}/${NAME}-challenge ${BARGS} ./challenge/
	rm -f challenge/challenge.tar
	docker save --output challenge/challenge.tar ${REGISTRY}/${NAME}-challenge
	#docker build -t ${REGISTRY}/${NAME}-build --target builder ${BARGS} .
	#docker run  -v ${MKPATH}/dist:/dist:z ${REGISTRY}/${NAME}-build
	#cp ${MKPATH}/challenge/challenge ${MKPATH}/solution/
	docker build -t ${REGISTRY}/${NAME} ${BARGS} .
	rm -f challenge/challenge.tar

ckill:
	@echo -e "\e[1;34m[+] Killing Challenge Container\e[0m"
	docker ps --filter "name=${NAME}" --format "{{.ID}}" \
		| tr '\n' ' ' \
		| xargs docker stop -t 0 \
		|| true

##########################
# Challenge Distribution #
##########################

# If tar complains about --transform, ensure you have GNU tar
dist: build
	id=$$(docker create ${REGISTRY}/${NAME}-challenge); \
	docker cp $${id}:/app/snow_globe dist/snow_globe; \
	docker cp $${id}:/app/globe_wrapper dist/globe_wrapper; \
	docker cp $${id}:/app/extractor dist/extractor; \
	docker rm -v $${id}; \

	chmod ug-s dist/snow_globe dist/extractor dist/globe_wrapper
	chown 1000:1000 dist/snow_globe dist/extractor dist/globe_wrapper

	envsubst < ./dist/deploy.sh.template | sed -e 's/§/$$/g' | \
		tee ./dist/deploy.sh >/dev/null
	chmod +x ./dist/deploy.sh
	cd dist && sha256sum * | sed -e '/sha256sum/d' -e '/deploy.sh.template/d' \
		-e '/.gitignore/d' | tee sha256sum
	tar -h --exclude 'deploy.sh.template' --exclude '.gitignore' \
		--transform s@^dist@${NAME}@ \
		--owner=huksys:1000 --group=huksys:1000 \
		-czvf ${NAME}.tar.gz dist/

# Deploy the generate dist file for testing
distrun: dist
	cd $(shell mktemp -d) && \
	cp ${MKPATH}/${NAME}.tar.gz . && \
	tar xf ${NAME}.tar.gz && \
	cd ${NAME} && \
	./deploy.sh

#########################
# Challenge Solvescript #
#########################

# Docker Run Args
SRARGS=-e HOST=${HOST} -e PORT=${PORT} -e DOMAIN=${DOMAIN} -e TIMEOUT=${TIMEOUT} \
			 --hostname localhost --net=host \
			 -v ${MKPATH}/challenge/flag.png:/app/flag.png:z,ro \
			 -v ${MKPATH}/dist/flag.png:/app/flag-fake.png:z,ro \
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

# Deployments is prepared for quadlets (containers + systemd services)
# https://www.redhat.com/sysadmin/quadlet-podman

deploy-privileged:
	@echo -e "\e[1;34m[+] Deploying Container/s\e[0m"
	envsubst < ./deployment/ctfd-entry.yml.template | \
		tee ctfd-${NAME}.yml >/dev/null
	sudo docker image scp ${USER}@localhost::${REGISTRY}/${NAME} root@localhost::
	envsubst < ./deployment/systemd-service.container.template | \
		sudo tee /etc/containers/systemd/chall-${NAME}.container >/dev/null
	sudo systemctl daemon-reload

# Assuming running user is the one that is operating this Makefile
deploy-unprivileged:
	@echo -e "\e[1;34m[+] Deploying Container/s\e[0m"
	envsubst < ./deployment/ctfd-entry.yml.template | \
		tee ctfd-${NAME}.yml >/dev/null
	mkdir -p ~/.config/containers/systemd/
	envsubst < ./deployment/systemd-service.container.template | \
		tee ~/.config/containers/systemd/chall-${NAME}.container >/dev/null
	systemctl --user daemon-reload

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
