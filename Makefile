IMAGE := mpreview-dev
BUILD_DIR := build
DOCKER_BUILD_DIR := build-jammy
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

.PHONY: all build check clean configure docker-build docker-test help image install rebuild run shell test

all: build

image:
	docker build -t $(IMAGE) -f docker/Dockerfile .

configure:
	cmake -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON

build: configure
	cmake --build $(BUILD_DIR) --parallel $(JOBS)

install: build
	DESTDIR="$(DESTDIR)" cmake --install $(BUILD_DIR) --prefix "$(PREFIX)"

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

docker-build:
	$(DOCKER_RUN) cmake -S . -B $(DOCKER_BUILD_DIR) -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
	$(DOCKER_RUN) cmake --build $(DOCKER_BUILD_DIR) --parallel $(JOBS)

docker-test: docker-build
	$(DOCKER_RUN) ctest --test-dir $(DOCKER_BUILD_DIR) --output-on-failure

check: test

run: build
	./$(BUILD_DIR)/mpreview "$(FILE)"

clean:
	@if [ -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		cmake --build $(BUILD_DIR) --target clean; \
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
	@echo "  make              Configure and compile for this host"
	@echo "  sudo make install Install the native preview and launcher"
	@echo "  make test         Compile and run tests on this host"
	@echo "  make run          Compile and preview README.md"
	@echo "  make run FILE=x   Compile and preview x"
	@echo "  make clean        Clean compiled artifacts via CMake"
	@echo "  make rebuild      Clean and compile"
	@echo "  make image        Build the optional Ubuntu 22.04 image"
	@echo "  make docker-build Compile in Docker (container use only)"
	@echo "  make docker-test  Run tests in Docker"
	@echo "  make shell        Open a shell in the development image"
