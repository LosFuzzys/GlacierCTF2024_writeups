# Ubuntu 24.04
# GCC 13.2.0
# Clang 18.0
# GLIBC 2.39

##################
# Builder system #
##################

# Building the linux kernel, buildroot and its modules is too complex for doing
# it here. Please build them separatedly and provide them in the challenge/
# folder

#################
# Runner system #
#################
FROM docker.io/library/ubuntu@sha256:3f85b7caad41a95462cf5b787d8a04604c8262cdcdf9a472b8c52ef83375fe15 as base

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y socat bash ruby libguestfs-tools openssh-client qemu-system-x86 python3 && apt-get clean 

# Copy kernel images
COPY challenge/buildroot/rootfs.qcow2 /app/
COPY challenge/linux/bzImage /app/

# Copy flag
COPY challenge/flag.txt /app/flag.txt

# Copy helper scripts
COPY challenge/helper/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh
COPY challenge/helper/cleaner.sh /app/cleaner.sh
RUN chmod +x /app/cleaner.sh
COPY challenge/helper/bootstrap.sh /app/bootstrap.sh
RUN chmod +x /app/bootstrap.sh

# Proof of work
COPY ./challenge/pow/pow /app/pow
RUN chmod +x /app/pow

USER 1000

CMD /app/bootstrap.sh 
