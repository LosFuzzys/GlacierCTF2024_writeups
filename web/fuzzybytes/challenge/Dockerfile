# Debian 12
# PHP 8.7.3
# Apache 2.4.59
FROM docker.io/library/php@sha256:b3ff205fcc739fc504750dab29d4c30afb2702730d37a1068a16c14f30a7d48f

RUN apt-get update && apt-get install -y python3 && apt-get clean

# Copy challenge required files
COPY ./config/php.ini $PHP_INI_DIR/php.ini
COPY ./web /var/www/html
COPY ./check_for_malicious_code.py /usr/
COPY ./flag.txt /root/flag.txt

RUN chown www-data:www-data /var/www/html/databases
RUN chmod +s /bin/tar
