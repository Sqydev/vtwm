MAKEFLAGS += --no-builtin-rules --warn-undefined-variables
.SUFFIXES:

# Flags
PROFILE ?= local

BASE_CFLAGS := -Wall -Wextra -Werror
DEV_CFLAGS  := -Og -g3 -fno-omit-frame-pointer -fsanitize=address,undefined
REL_CFLAGS  := -O2

LIBC ?= glibc

CC ?= gcc
STD ?= -std=c99
POSIX_C_SRC ?= -D_POSIX_C_SOURCE=200809L

X11_LDFLAGS := -lX11 -lXinerama

# Xephyr test env
XEPHYR_DISPLAY ?= :1
XEPHYR_RES     ?= 800x600

ifeq ($(LIBC),static-musl)
  LIBC_CFLAGS  := -static
  LIBC_LDFLAGS := -static
else
  LIBC_CFLAGS  :=
  LIBC_LDFLAGS :=
endif

CFLAGS += $(BASE_CFLAGS) $(REL_CFLAGS) $(LIBC_CFLAGS) $(STD) $(POSIX_C_SRC)
LDFLAGS ?= $(LIBC_LDFLAGS)

# Paths
SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := compiled
TARGET  := vtwm

OBJ_SUBDIR := $(OBJ_DIR)/$(PROFILE)-$(LIBC)

ifeq ($(filter $(PROFILE),san check),)
  BIN_SUBDIR := $(PROFILE)
else
  BIN_SUBDIR := test
endif

OUT := $(BIN_DIR)/$(BIN_SUBDIR)/$(TARGET)-$(PROFILE)-$(LIBC)

# Sources
SRC := $(sort $(shell find $(SRC_DIR) -name '*.c'))
OBJ := $(patsubst $(SRC_DIR)/%.c,$(OBJ_SUBDIR)/%.o,$(SRC))
DEP := $(OBJ:.o=.d)

.PHONY: all release install upgrade update build local-build san-build check-build \
        test-build wm run-xsession \
        docker-bleeding docker-normal docker-stable \
        docker-bleeding-musl docker-normal-musl docker-stable-musl \
        docker-static-musl clean clean-all

all: local-build san-build check-build

PREFIX  ?=
DESTDIR ?=

install: build
	install -Dm755 $(OUT) $(DESTDIR)$(PREFIX)/bin/$(TARGET)
	install -Dm644 vtwm.desktop $(DESTDIR)/usr/share/xsessions/vtwm.desktop

upgrade: update build install

update:
	git fetch origin
	git merge --ff-only origin/main

build: $(OUT)

local-build:
	$(MAKE) PROFILE=local LIBC=glibc CFLAGS="$(BASE_CFLAGS) $(REL_CFLAGS)" LDFLAGS="" build

san-build:
	$(MAKE) PROFILE=san LIBC=glibc CFLAGS="$(BASE_CFLAGS) $(DEV_CFLAGS)" \
		LDFLAGS="-fsanitize=address,undefined $(X11_LDFLAGS)" build

check-build:
	$(MAKE) PROFILE=check LIBC=glibc CFLAGS="$(BASE_CFLAGS) $(REL_CFLAGS)" build

test-build: san-build check-build

wm: run-xsession

run-xsession: build
	@exec startx exec compiled/local/vtwm-local-glibc --

$(OUT): $(OBJ)
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(OBJ) -o $@ $(LDFLAGS) $(X11_LDFLAGS)

$(OBJ_SUBDIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

-include $(DEP)

UID := $(shell id -u)
GID := $(shell id -g)
DOCKER_USER := --rm -u $(UID):$(GID) -v "$(shell pwd)":/src -w /src
DOCKER_ROOT := --rm -v "$(shell pwd)":/src -w /src
FIX_PERMS   := ; chown -R $(UID):$(GID) $(OBJ_DIR) $(BIN_DIR)

docker-bleeding:
	docker run $(DOCKER_USER) gcc:latest $(MAKE) PROFILE=bleeding LIBC=glibc build

docker-normal:
	docker run $(DOCKER_USER) gcc:11 $(MAKE) PROFILE=normal LIBC=glibc build

docker-stable:
	docker run $(DOCKER_USER) gcc:7 $(MAKE) PROFILE=stable LIBC=glibc build

docker-bleeding-musl:
	docker run $(DOCKER_ROOT) alpine:latest sh -c "apk add --no-cache build-base musl-dev && $(MAKE) PROFILE=bleeding LIBC=musl build $(FIX_PERMS)"

docker-normal-musl:
	docker run $(DOCKER_ROOT) alpine:latest sh -c "apk add --no-cache build-base musl-dev && $(MAKE) PROFILE=normal LIBC=musl build $(FIX_PERMS)"

docker-stable-musl:
	docker run $(DOCKER_ROOT) alpine:3.19 sh -c "apk add --no-cache build-base musl-dev && $(MAKE) PROFILE=stable LIBC=musl build $(FIX_PERMS)"

docker-static-musl:
	docker run $(DOCKER_ROOT) alpine:3.19 sh -c "apk add --no-cache build-base musl-dev && $(MAKE) PROFILE=static LIBC=static-musl build $(FIX_PERMS)"

clean-all:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

clean:
	rm -rf $(OBJ_SUBDIR)
