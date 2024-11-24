# Ubuntu 22.04.4 LTS
# SageMath version 10.3 stable
# Python 3.11.1
FROM docker.io/sagemath/sagemath@sha256:aeefedc716b4f475ddc5f8d8140dbb231868b8a2e5882044845c83d41d1014a5

# Run as root
USER 0

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y python3 coreutils && apt-get clean

# Enable the virtual enviroment
#RUN python3 -m venv /opt/venv
#ENV PATH="/opt/venv/bin:$PATH"

COPY challenge /app/challenge
WORKDIR /app

# Install pip dependencies via sage
COPY requirements.txt .
RUN sage -pip install --no-cache-dir --upgrade -r /app/requirements.txt 

COPY exploit .
USER root
RUN chmod 755 /app/exploit
USER 1000

ENV DOT_SAGE=/tmp/

ENV PWNLIB_NOTERM=1

CMD "/usr/bin/timeout ${TIMEOUT} ./exploit HOST=${HOST} PORT=${PORT}"
