# Debian sid (need llvm-19 for latest zig)

###############
# Base system #
###############
FROM docker.io/library/debian:bookworm-slim@sha256:e30724a77d3b0e52ea510342a0424895e810edd96423b5555965307142da032b AS base

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y strace wget xz-utils && apt-get clean

RUN wget https://ziglang.org/builds/zig-linux-x86_64-0.14.0-dev.2271+f845fa04a.tar.xz -O /tmp/zig.tar.xz &&\
    mkdir /tmp/zig &&\
    tar xf /tmp/zig.tar.xz -C /tmp/zig &&\
    mv /tmp/zig/zig-linux-x86_64-0.14.0-dev* /tmp/zig/zig-linux-x86_64-0.14.0-dev &&\
    cp /tmp/zig/zig-linux-x86_64-0.14.0-dev/zig /usr/local/bin/zig &&\
    cp -r /tmp/zig/zig-linux-x86_64-0.14.0-dev/lib /usr/local/lib/zig &&\
    rm -rf /tmp/zig /tmp/zig.tar.xz


RUN mkdir /app
COPY src /app/src
COPY flag.txt /flag.txt
COPY build.zig /app/

# Build in /tmp/app, so that the .zig-cache is vaild within the jail build
RUN cp -r /app /tmp
WORKDIR /tmp/app
RUN zig build --global-cache-dir /tmp/app/.zig-cache &&\
    cp -r /tmp/app/.zig-cache /app/.zig-cache &&\
    rm /app/src/notes.json &&\
    rm -rf /tmp/app


#################
# Runner system #
#################
# Opensuse Tumbleweed (only for running nsjail)
FROM docker.io/opensuse/tumbleweed@sha256:b2daeb56bbe4edcc696af872ed0ad3d2ad0120e3fc0e06c791f16dcb5bed15ad 

# Install apk dependencies if needed
RUN zypper -n install nsjail && zypper -n clean --all

# Copy base filesystem
COPY --from=base / /jail

# Dirty hack to make zig find it's own executable
RUN mkdir -p /jail/proc/self &&\
    ln -s /usr/local/bin/zig /jail/proc/self/exe

RUN mkdir -p /jail/app
COPY entrypoint.sh /jail/entrypoint.sh

# add "-m none:/DESTPATH:tmpfs:size=N" before --cwd on nsjail args to have a tmpfs-backed writable DESTPATH of N bytes
# remember that /DESTPATH cannot contain any files coming from /jail (as its a mount). If you want 
# pre-created/copied files in /DESTPATH you should manually copy them in entrypoint.sh
# Note: /DESTPATH should not contain /jail as a prefix
CMD nsjail --mode l --disable_proc --time_limit ${TIMEOUT} --rlimit_nofile=128 --rlimit_fsize=100000000 --bindhost 0.0.0.0 --port 1337 --bindmount_ro /jail:/ -m none:/dev:tmpfs:mode=555,size=1,uid=65534,gid=65534 -R /dev/urandom -R /dev/random -B /dev/null -R /dev/zero \
  -m none:/tmp:tmpfs:size=100000000 --cwd /app/ -u 1337:1337:1 -g 1337:1337:1 -u 65534:65534:1 -g 65534:65534:1 -- /entrypoint.sh

