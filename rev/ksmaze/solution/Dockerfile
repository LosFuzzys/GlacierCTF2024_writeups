# - Ubuntu 22.04.4 LTS
# - pwntools v4.12.0 stable
# - Python 3.10.12
# - GLIBC 2.35-0ubuntu3.6
FROM docker.io/pwntools/pwntools@sha256:2df32ff2a7196a6f6d362b037713ec9825266667774df31540bbb9d04f7662ee

# Install apt dependencies if needed
USER root
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y tmux hashcash gcc openssh-client && apt-get clean
USER pwntools

WORKDIR /app
COPY requirements.txt .

# Install pip dependencies
RUN python3 -m pip install --no-cache-dir --upgrade -r /app/requirements.txt

# Copy needed files to /app
COPY exploit .
COPY tmuxio.py .
USER root
RUN chmod 755 /app/exploit
USER pwntools

ENV PWNLIB_NOTERM=1

CMD /usr/bin/timeout -k 5 ${TIMEOUT} ./exploit HOST=${HOST} PORT=${PORT}
