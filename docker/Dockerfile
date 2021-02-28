# MDV service Dockerfile
# build example from repo root folder:
# docker build -t nilyin/medveddb:1.22-debian11 -f ./docker/Dockerfile .
# run MDB server service example with named volume @ host:
# docker run -d -p 4800:4800 -v medveddb:/opt/medveddb/data:rw  --restart=unless-stopped --name=mdb [IMAGE_ID]
# run for host medveddb client example , use \? for help when prompted:
# ./mdv tcp://localhost:4800


FROM debian:bullseye-20210208-slim
LABEL MAINTAINER="Nikolay ILYIN <ni@peerf.me>"
LABEL name="medveddb"

RUN apt-get update -q -q \
&& apt-get upgrade --yes \
&& apt-get install -yq apt-utils \
&& apt-get install -yq --no-install-recommends \
           systemd sudo less \
&& apt-get clean && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/medveddb/data
RUN useradd --user-group --system --shell /bin/bash --create-home --no-log-init medved
WORKDIR /opt/medveddb

LABEL version="1.22"

COPY ./build/mdv_service/medved ./build/mdv_client/mdv ./docker/medved.conf ./

# change to the user, created by installing the package medveddb
RUN chown -R medved:medved /opt/medveddb
USER medved

# Espose default MDV port
EXPOSE 4800

# optionally add VOLUME to allow to persist database data. We skip it to avoid of anonymous volume creation
# VOLUME ["/opt/medveddb/data"]

# Finally launch medveddb service.
ENTRYPOINT ["/opt/medveddb/medved", "--cfg=/opt/medveddb/medved.conf"]