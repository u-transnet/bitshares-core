# This will build the witness_node in a docker image. Make sure you've already
# checked out the submodules before building.

FROM ubuntu:16.04

RUN apt-get update \
    && apt-get install -y autoconf cmake git libboost-all-dev libssl-dev g++

ADD . /bitshares-2
WORKDIR /bitshares-2
RUN cmake -DCMAKE_BUILD_TYPE=RelWithDebInfo .
RUN make witness_node

RUN mkdir /data_dir
ADD docker/default_config.ini /default_config.ini
ADD docker/launch /launch
RUN chmod a+x /launch
VOLUME /data_dir

EXPOSE 8090 9090

ENTRYPOINT ["/launch"]
