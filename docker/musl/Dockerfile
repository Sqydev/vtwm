FROM alpine:latest

RUN apk add --no-cache \
    build-base \
    cmake \
    make \
    pkgconf \
    musl-dev \
    wlroots0.20-dev \
    wayland-dev \
    wayland-protocols \
    libxkbcommon-dev \
    pixman-dev \
    libinput-dev

WORKDIR /src