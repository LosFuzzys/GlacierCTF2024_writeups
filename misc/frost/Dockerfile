# Ubuntu 24.04

###############
# Base system #
###############
FROM docker.io/library/ubuntu@sha256:3f85b7caad41a95462cf5b787d8a04604c8262cdcdf9a472b8c52ef83375fe15 AS base

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y python3 python3-venv socat coreutils gfortran meson python3-pip python3-setuptools python3-wheel ninja-build && apt-get clean

# Enable the virtual enviroment
RUN python3 -m venv /.venv
ENV PATH="/.venv/bin:$PATH"

# Install pip dependencies
COPY ./challenge/requirements.txt /app/requirements.txt
RUN pip install --no-cache-dir --upgrade -r /app/requirements.txt 

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
COPY challenge/challenge /jail/app/challenge
RUN chmod +x /jail/app/challenge
COPY challenge/flag.txt /jail/flag.txt
COPY challenge/entrypoint.sh /jail/app/entrypoint.sh
COPY challenge/meson.build /jail/app/meson.build
RUN mkdir -p /jail/build

# add "-m none:/DESTPATH:tmpfs:size=N" before --cwd on nsjail args to have a tmpfs-backed writable DESTPATH of N bytes
# remember that /DESTPATH cannot contain any files coming from /jail (as its a mount). If you want 
# pre-created/copied files in /DESTPATH you should manually copy them in entrypoint.sh
# Note: /DESTPATH should not contain /jail as a prefix
CMD nsjail --mode l --disable_proc --time_limit ${TIMEOUT} --bindhost 0.0.0.0 --port 1337 --bindmount_ro /jail:/ -m none:/build:tmpfs:size=5000000 -m none:/dev:tmpfs:mode=555,size=1,uid=65534,gid=65534 -R /dev/urandom -R /dev/random -B /dev/null -R /dev/zero --cwd /app/ -u 1337:1337:1 -g 1337:1337:1 -u 65534:65534:1 -g 65534:65534:1 -- /app/entrypoint.sh
