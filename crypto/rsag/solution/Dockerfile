# Ubuntu 22.04.4 LTS
# SageMath version 10.3 stable
# Python 3.11.1

###############
# Base system #
###############
FROM docker.io/sagemath/sagemath@sha256:aeefedc716b4f475ddc5f8d8140dbb231868b8a2e5882044845c83d41d1014a5

# Run as root
USER 0

# Install apt dependencies if needed
RUN DEBIAN_FRONTEND=noninteractive apt-get update && apt-get install -y python3 python3-venv && apt-get clean

RUN sage --python3 -m venv /home/sage/.venv
ENV PATH="/home/sage/.venv/bin:$PATH"

# Install pip dependencies
COPY requirements.txt requirements.txt
RUN sage -pip install --no-cache-dir --upgrade -r requirements.txt 

# Copy challenge required files
COPY solve.sage solve.sage

# Quoting is required in this template
CMD "sage solve.sage"
