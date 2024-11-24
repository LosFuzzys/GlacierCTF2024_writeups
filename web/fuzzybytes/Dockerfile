# Ubuntu 24.04
# GCC 13.2.0
# Clang 18.0
# GLIBC 2.39

##################
# Builder system #
##################

# Builds are complex and its better to manage them separatedly than to include
# buildroot & linux-kernel into docker

#################
# Runner system #
#################
FROM quay.io/podman/stable:v5.2.0 as base

# Install dnf dependencies if needed
RUN dnf -y install socat caddy iproute && dnf clean all

#RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y socat podman fuse-overlayfs curl jq && apt-get clean 

# Copy the actual challenge container image
COPY challenge/challenge.tar /app/
RUN chown 1000:1000 /app/challenge.tar

COPY helper/Caddyfile /app/Caddyfile

# Copy helper scripts
COPY helper/run.sh /app/run.sh
RUN chmod +x /app/run.sh
COPY helper/cleaner.sh /app/cleaner.sh
RUN chmod +x /app/cleaner.sh
COPY helper/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh

USER 1000

CMD /app/entrypoint.sh 
