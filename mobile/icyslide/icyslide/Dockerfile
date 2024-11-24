# Alpine 3.19.1 (base system)

###############
# Base system #
###############
FROM docker.io/library/alpine@sha256:c5b1261d6d3e43071626931fc004f70149baeba2c8ec672bd4f27761f8e1ad6b

# Install apt dependencies if needed
RUN apk update && apk add --no-cache socat python3 py3-pip coreutils && rm -rf /var/cache/apk/*

# Enable the virtual enviroment
RUN python3 -m venv /.venv
ENV PATH="/.venv/bin:$PATH"

# Install pip dependencies
COPY challenge/requirements.txt /app/requirements.txt
RUN pip install --no-cache-dir --upgrade -r /app/requirements.txt 

# Copy challenge required files
RUN mkdir -p /app
COPY challenge/challenge /app/challenge
RUN chmod +x /app/challenge
COPY challenge/flag.txt /flag.txt
COPY challenge/entrypoint.sh /app/entrypoint.sh

CMD /app/entrypoint.sh
