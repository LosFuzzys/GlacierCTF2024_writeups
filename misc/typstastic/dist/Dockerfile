# Alpine 3.19.1 (base system)

###############
# Base system #
###############
FROM docker.io/library/alpine@sha256:c5b1261d6d3e43071626931fc004f70149baeba2c8ec672bd4f27761f8e1ad6b AS base

# Install apk dependencies if needed
RUN apk update && apk add --no-cache --repository=https://dl-cdn.alpinelinux.org/alpine/edge/community typst

# Install apt dependencies if needed
RUN apk update && apk add --no-cache coreutils file && rm -rf /var/cache/apk/*

#################
# Runner system #
#################
# Opensuse Tumbleweed (only for running nsjail)
FROM docker.io/opensuse/tumbleweed@sha256:b2daeb56bbe4edcc696af872ed0ad3d2ad0120e3fc0e06c791f16dcb5bed15ad 

# Install apk dependencies if needed
RUN zypper -n install nsjail fakeroot && zypper -n clean --all

# Copy base filesystem
COPY --from=base / /jail

# Copy challenge required files
RUN mkdir -p /jail/app
COPY challenge /jail/app/challenge
RUN chmod +x /jail/app/challenge
COPY flag.txt /jail/flag.txt
COPY entrypoint.sh /jail/app/entrypoint.sh

CMD nsjail --mode l --disable_proc --time_limit ${TIMEOUT} --bindhost 0.0.0.0 --port 1337 --bindmount_ro /jail:/ -m none:/dev:tmpfs:mode=555,size=1,uid=65534,gid=65534 -R /dev/urandom -R /dev/random -B /dev/null -R /dev/zero -m none:/tmp:tmpfs:size=5242880 --cwd /app/ -u 1337:1337:1 -g 1337:1337:1 -u 65534:65534:1 -g 65534:65534:1 -- /app/entrypoint.sh
