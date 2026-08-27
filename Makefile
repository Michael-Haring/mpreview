IMAGE := mpreview-dev
BUILD_DIR := build-jammy
FILE ?= README.md
JOBS ?= $(shell nproc)
PREFIX ?= /usr/local
DESTDIR ?=

DOCKER_RUN := docker run --rm \
	--user "$(shell id -u):$(shell id -g)" \
	-v "$(CURDIR):/workspace" \
	-w /workspace \
	$(IMAGE)

.DEFAULT_GOAL := build

.PHONY: all build check clean configure help image install rebuild run shell test

all: build

image:
	docker build -t $(IMAGE) -f docker/Dockerfile .

configure:
	$(DOCKER_RUN) cmake -S . -B $(BUILD_DIR) -DBUILD_TESTING=ON

build: configure
	$(DOCKER_RUN) cmake --build $(BUILD_DIR) --parallel $(JOBS)

install: build
	install -D -m 755 "mpreview.sh" "$(DESTDIR)$(PREFIX)/bin/mpreview"
	install -D -m 755 "$(BUILD_DIR)/mpreview" \
		"$(DESTDIR)$(PREFIX)/lib/mpreview/mpreview"

test: build
	$(DOCKER_RUN) ctest --test-dir $(BUILD_DIR) --output-on-failure

check: test

run: build
	./mpreview.sh "$(FILE)"

clean:
	@if [ -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		$(DOCKER_RUN) cmake --build $(BUILD_DIR) --target clean; \
	else \
		echo "Nothing to clean."; \
	fi

rebuild: clean build

shell:
	docker run --rm -it \
		--user "$(shell id -u):$(shell id -g)" \
		-v "$(CURDIR):/workspace" \
		-w /workspace \
		$(IMAGE) bash

help:
	@echo "mpreview development targets:"
	@echo "  make              Configure and compile"
	@echo "  sudo make install Install to /usr/local/bin/mpreview"
	@echo "  make test         Compile and run tests"
	@echo "  make run          Compile and preview README.md"
	@echo "  make run FILE=x   Compile and preview x"
	@echo "  make clean        Clean compiled artifacts via CMake"
	@echo "  make rebuild      Clean and compile"
	@echo "  make image        Rebuild the Ubuntu 22.04 image"
	@echo "  make shell        Open a shell in the development image"
