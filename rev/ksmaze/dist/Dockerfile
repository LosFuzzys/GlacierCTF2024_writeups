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
#
# challenge/linux/bzImage
# challenge/buildroot/rootfs.qcow2
#
# You can use qemu-img to convert raw ext2 to qcow:
# qemu-img convert -f raw -O qcow2 ./challenge/buildroot/rootfs.ext2 ./challenge/buildroot/rootfs.qcow2

#################
# Runner system #
#################
FROM docker.io/library/ubuntu@sha256:3f85b7caad41a95462cf5b787d8a04604c8262cdcdf9a472b8c52ef83375fe15 as base

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y socat bash ruby libguestfs-tools openssh-client qemu-system-x86 && apt-get clean 

# Copy kernel images & modules
COPY rootfs.qcow2 /app/
COPY bzImage /app/

# Copy flag
COPY flag.txt /app/flag.txt

# Copy helper scripts
COPY helper/entrypoint.sh /app/entrypoint.sh
RUN chmod +x /app/entrypoint.sh
COPY helper/cleaner.sh /app/cleaner.sh
RUN chmod +x /app/cleaner.sh
COPY helper/bootstrap.sh /app/bootstrap.sh
RUN chmod +x /app/bootstrap.sh

# Proof of work
COPY pow/pow /app/pow
RUN chmod +x /app/pow

USER 1000

CMD /app/bootstrap.sh 
