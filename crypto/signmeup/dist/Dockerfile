# Ubuntu 24.04
# GCC 13.2.0
# Clang 18.0
# GLIBC 2.39

##################
# Builder system #
##################
#FROM docker.io/library/ubuntu@sha256:3f85b7caad41a95462cf5b787d8a04604c8262cdcdf9a472b8c52ef83375fe15 AS builder
#
## Install apt dependencies if needed
#RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y cargo make gcc && apt-get clean 
#
#COPY challenge/ /tmp/build/
#RUN make -C /tmp/build/ challenge
## Copy final binary to dist
## CACHEBUST is a Docker trick to invalidate the cache at a exact point
## if not, when docker has cached the compilation it wouldn't output
## the binary to our dist/ directory so we can pack it for distribution
#ARG CACHEBUST=1
#CMD cp /tmp/build/challenge /dist/challenge

#################
# Runner system #
#################
FROM docker.io/library/ubuntu@sha256:3f85b7caad41a95462cf5b787d8a04604c8262cdcdf9a472b8c52ef83375fe15

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y socat coreutils && apt-get clean 

# Copy challenge required files
RUN mkdir -p /app
#COPY --from=builder /tmp/build/challenge /app/challenge
COPY challenge /app/challenge
RUN chmod +x /app/challenge
COPY flag.txt /flag.txt
COPY entrypoint.sh /app/entrypoint.sh

CMD socat -T 60 TCP-LISTEN:1337,fork,nodelay,reuseaddr,pktinfo EXEC:"/usr/bin/timeout -k 5 ${TIMEOUT} /app/entrypoint.sh"
