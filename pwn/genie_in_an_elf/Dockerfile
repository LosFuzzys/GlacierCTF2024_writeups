# Ubuntu 24.04
# GCC 13.2.0
# Clang 18.0
# GLIBC 2.39

##################
# Builder system #
##################
FROM docker.io/library/ubuntu@sha256:3f85b7caad41a95462cf5b787d8a04604c8262cdcdf9a472b8c52ef83375fe15 AS builder

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y make gcc && apt-get clean 

COPY challenge/ /tmp/build/
RUN make -C /tmp/build/ challenge
# Copy final binary to dist
# CACHEBUST is a Docker trick to invalidate the cache at a exact point
# if not, when docker has cached the compilation it wouldn't output
# the binary to our dist/ directory so we can pack it for distribution
ARG CACHEBUST=1
CMD cp /tmp/build/challenge /dist/challenge && cp /lib/x86_64-linux-gnu/libc.so.6 /dist/ && cp /lib64/ld-linux-x86-64.so.2 /dist/ && echo "c1e50a701d3245c8c45a1ba40151efe414ac41d2c9ae604087b4875e0d85c4fb  /dist/libc.so.6" | sha256sum -c

#################
# Runner system #
#################
FROM docker.io/library/ubuntu@sha256:3f85b7caad41a95462cf5b787d8a04604c8262cdcdf9a472b8c52ef83375fe15

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y socat coreutils && apt-get clean 

# Copy challenge required files
RUN mkdir -p /app
COPY --from=builder /tmp/build/challenge /app/challenge
RUN chmod +x /app/challenge
COPY challenge/flag.txt /flag.txt
COPY challenge/entrypoint.sh /app/entrypoint.sh

CMD socat -T 60 TCP-LISTEN:1337,fork,nodelay,reuseaddr,pktinfo EXEC:"/usr/bin/timeout -k 5 ${TIMEOUT} /app/entrypoint.sh"
