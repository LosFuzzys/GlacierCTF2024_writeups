FROM docker.io/library/python@sha256:1b8e511c3a5b60fdecce05282a196f23a60d487a8838df4a711b5d83a7d58fc9

# Copy needed files to /app
WORKDIR /app
COPY reconstrct.py .
COPY replica_1_challenge.bin .
COPY replica_2_challenge.bin .
COPY replica_3_challenge.bin .

CMD python3 reconstrct.py && chmod +x reconstructed.bin && ./reconstructed.bin
