# Python3.12-Bookworm Container
# Debian Bookworm (Basesystem) with Python3.12 installed 

###############
# Base system #
###############
FROM docker.io/library/python@sha256:db1afbfdc089b792bed227d70e1b1607c16bacb391f1257b7af1f0e6b2384480

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update \
    && apt-get install -y curl git \
    && apt-get clean \
    && python -m pip install pwntools

# Install Foundary
RUN curl -L https://foundry.paradigm.xyz | bash \
    && bash -c "\
        source /root/.bashrc \
        && foundryup \
            --version nightly-a428ba6ad8856611339a6319290aade3347d25d9 \
            --arch amd64" \
    && chmod 755 -R /root

WORKDIR /tmp/solution
COPY foundry.toml .
COPY solve_challenge.py .
COPY sources sources/.

CMD /usr/bin/timeout -k 5 ${TIMEOUT} python solve_challenge.py \
    --host ${HOST} --port ${PORT} --ticket ${TICKET} ${VERBOSE}
