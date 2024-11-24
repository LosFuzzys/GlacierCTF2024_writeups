# Alpine 3.19.1

##################
# Builder system #
##################
FROM docker.io/library/alpine@sha256:c5b1261d6d3e43071626931fc004f70149baeba2c8ec672bd4f27761f8e1ad6b AS builder

# Install apk dependencies if needed
RUN apk update && apk add --no-cache cargo make && rm -rf /var/cache/apk/*

COPY challenge/ /tmp/build/
RUN make -C /tmp/build/ challenge
# Copy final binary to dist
# CACHEBUST is a Docker trick to invalidate the cache at a exact point
# if not, when docker has cached the compilation it wouldn't output
# the binary to our dist/ directory so we can pack it for distribution
ARG CACHEBUST=1
CMD cp /tmp/build/challenge-dist /dist/challenge

#################
# Runner system #
#################
FROM docker.io/library/alpine@sha256:c5b1261d6d3e43071626931fc004f70149baeba2c8ec672bd4f27761f8e1ad6b

# Install apk dependencies if needed
RUN apk update && apk add --no-cache socat libgcc coreutils libc6-compat && rm -rf /var/cache/apk/*

# Copy challenge required files
RUN mkdir -p /app
COPY --from=builder /tmp/build/challenge /app/challenge
RUN chmod +x /app/challenge
COPY challenge/flag.txt /flag.txt
COPY challenge/flag.txt /app/flag.txt
COPY challenge/entrypoint.sh /app/entrypoint.sh
COPY challenge/config/mechanics.yml /app/mechanics.yml

CMD /app/entrypoint.sh
