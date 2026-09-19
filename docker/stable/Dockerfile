FROM gcc:12

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        cmake \
        make \
        pkg-config \
        libwlroots-dev \
        libwayland-dev \
        wayland-protocols \
        libxkbcommon-dev \
        libpixman-1-dev \
        libinput-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src