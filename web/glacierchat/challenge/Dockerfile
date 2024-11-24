FROM php:8.2-apache-bookworm
RUN apt update -y && apt install -y git && apt clean

WORKDIR /var/www/

COPY ./config/php.ini $PHP_INI_DIR/php.ini
COPY ./web/ .
COPY ./entrypoint.sh /
COPY ./service/ /service
COPY ./flag.txt /
RUN chmod 640 /flag.txt

COPY --from=docker.io/composer:2.7.7 /usr/bin/composer /usr/local/bin/composer
RUN /usr/local/bin/composer install

RUN chown -R www-data:www-data /var/www/

CMD ["/entrypoint.sh"]
