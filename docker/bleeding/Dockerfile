FROM gcc:latest

ARG WLROOTS_VERSION=0.19.3

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        cmake \
        make \
        pkg-config \
        git \
        meson \
        ninja-build \
        libwayland-dev \
        wayland-protocols \
        libdrm-dev \
        libgbm-dev \
        libegl-dev \
        libgles2 \
        libinput-dev \
        libxkbcommon-dev \
        libpixman-1-dev \
        libudev-dev \
        libseat-dev \
        libliftoff-dev \
        libdisplay-info-dev \
        hwdata \
    && rm -rf /var/lib/apt/lists/* \
    && git clone --depth 1 --branch ${WLROOTS_VERSION} \
        https://gitlab.freedesktop.org/wlroots/wlroots.git /wlroots \
    && meson setup /wlroots/build /wlroots \
        -Dexamples=false -Dxwayland=disabled \
    && ninja -C /wlroots/build install \
    && ldconfig \
    && rm -rf /wlroots

WORKDIR /src