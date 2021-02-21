# MDV service Dockerfile
# run example:
# docker run -d -p 4800:4800 -v /opt/data:/opt/data:rw  --restart=unless-stopped --name=mdb [IMAGE_ID]


FROM debian:bullseye-20210208-slim
LABEL MAINTAINER="Nikolay ILYIN <ni@peerf.me>"
LABEL name="medveddb"

RUN apt-get update -q -q \
&& apt-get upgrade --yes \
&& apt-get install -yq apt-utils \
&& apt-get install -yq --no-install-recommends \
           systemd sudo less \
&& apt-get clean && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/medveddb /opt/data
RUN useradd --user-group --system --shell /bin/bash --create-home --no-log-init medved
WORKDIR /opt/medveddb

LABEL version="1.2"

COPY medved medved.conf ./

# change to the user, created by installing the package medveddb
RUN chown -R medved:medved /opt
USER medved

# Espose default MDV port
EXPOSE 4800

# Add VOLUMEs to allow backup of config, logs and databases
VOLUME ["/opt/data"]

# Finally launch medveddb service.
ENTRYPOINT ["/opt/medveddb/medved", "--cfg=/opt/medveddb/medved.conf"]