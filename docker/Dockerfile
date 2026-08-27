FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        cmake \
        libcmark-gfm-dev \
        libcmark-gfm-extensions-dev \
        libgtk-4-dev \
        libwebkitgtk-6.0-dev \
        pkg-config \
    && apt-get clean

WORKDIR /workspace

CMD ["bash"]
