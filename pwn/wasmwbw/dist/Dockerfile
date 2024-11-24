# Ubuntu 24.04
# GCC 13.2.0
# Clang 18.0
# GLIBC 2.39

##################
# Builder system #
##################
FROM docker.io/library/ubuntu@sha256:3f85b7caad41a95462cf5b787d8a04604c8262cdcdf9a472b8c52ef83375fe15 AS builder

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y make curl gcc && apt-get clean

COPY ./ /tmp/build/

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- --default-toolchain none -y &&\
    . $HOME/.cargo/env &&\
    rustup toolchain install stable --profile minimal &&\
    rustup default stable

RUN . $HOME/.cargo/env && make -C /tmp/build/ challenge
# Copy final binary to dist
# CACHEBUST is a Docker trick to invalidate the cache at a exact point
# if not, when docker has cached the compilation it wouldn't output
# the binary to our dist/ directory so we can pack it for distribution
ARG CACHEBUST=1
CMD cp /tmp/build/challenge challenge_dist

###############
# Base system #
###############
FROM docker.io/library/ubuntu@sha256:3f85b7caad41a95462cf5b787d8a04604c8262cdcdf9a472b8c52ef83375fe15 AS base

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y coreutils && apt-get clean

#################
# Runner system #
#################
# Opensuse Tumbleweed (only for running nsjail)
FROM docker.io/opensuse/tumbleweed@sha256:b2daeb56bbe4edcc696af872ed0ad3d2ad0120e3fc0e06c791f16dcb5bed15ad 

# Install apk dependencies if needed
RUN zypper -n install nsjail && zypper -n clean --all

# Copy base filesystem
COPY --from=base / /jail

# Copy challenge required files
RUN mkdir -p /jail/app
COPY --from=builder /tmp/build/challenge /jail/app/challenge
RUN chmod +x /jail/app/challenge
COPY flag.txt /jail/flag.txt
COPY entrypoint.sh /jail/app/entrypoint.sh

# add "-m none:/DESTPATH:tmpfs:size=N" before --cwd on nsjail args to have a tmpfs-backed writable DESTPATH of N bytes
# remember that /DESTPATH cannot contain any files coming from /jail (as its a mount). If you want 
# pre-created/copied files in /DESTPATH you should manually copy them in entrypoint.sh
# Note: /DESTPATH should not contain /jail as a prefix
CMD nsjail --mode l --proc_rw --time_limit ${TIMEOUT} --bindhost 0.0.0.0 --port 1337 --bindmount /proc:/proc --bindmount_ro /jail:/ -m none:/dev:tmpfs:mode=555,size=1,uid=65534,gid=65534 -R /dev/urandom -R /dev/random -B /dev/null -R /dev/zero --cwd /app/ -u 1337:1337:1 -g 1337:1337:1 -u 65534:65534:1 -g 65534:65534:1 -- /app/entrypoint.sh
