# Ubuntu 24.04

###############
# Base system #
###############
FROM docker.io/library/ubuntu@sha256:3f85b7caad41a95462cf5b787d8a04604c8262cdcdf9a472b8c52ef83375fe15 AS builder

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y python3 python3-venv socat coreutils redis && apt-get clean

# Enable the virtual enviroment
RUN python3 -m venv /.venv
ENV PATH="/.venv/bin:$PATH"

# Install pip dependencies
COPY challenge/requirements.txt /app/requirements.txt
RUN pip install --no-cache-dir --upgrade -r /app/requirements.txt 
RUN pip install playwright==1.47.0 rq==1.15.1
RUN playwright install --with-deps chromium

# Copy challenge required files
RUN mkdir -p /app
COPY challenge/src /app/src
COPY challenge/flag.txt /flag.txt
COPY challenge/entrypoint.sh /app/entrypoint.sh

CMD /app/entrypoint.sh
