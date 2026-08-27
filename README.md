# mpreview

`mpreview` is a small native Linux Markdown previewer. It renders GitHub-Flavored
Markdown in a GTK window and reloads automatically when the source file is saved.

## Dependencies

On Ubuntu 22.04 or newer:

```bash
sudo apt update
sudo apt install build-essential cmake pkg-config libgtk-4-dev \
  libwebkitgtk-6.0-dev libcmark-gfm-dev libcmark-gfm-extensions-dev
```

The `universe` repository may need to be enabled for the WebKitGTK and cmark-gfm
development packages.

## Build and install

```bash
cmake -S . -B build
cmake --build build
sudo cmake --install build
```

### Containerized development

To keep development libraries off the host, build the Ubuntu 22.04 development image:

```bash
docker build -t mpreview-dev -f docker/Dockerfile .
```

The usual development commands are wrapped by the Makefile:

```bash
make
make test
make run
make run FILE=docs/design.md
make clean
make rebuild
make image
```

Configure and compile in an ephemeral container. Passing the host user and group IDs
keeps generated files owned by the current user:

```bash
docker run --rm --user "$(id -u):$(id -g)" \
  -v "$PWD:/workspace" -w /workspace mpreview-dev \
  bash -lc 'cmake -S . -B build-jammy && cmake --build build-jammy'
```

Run the preview through the included launcher:

```bash
./mpreview.sh README.md
```

Tile or maximize the preview at startup:

```bash
./mpreview.sh --right README.md
./mpreview.sh --left README.md
./mpreview.sh --maximized README.md
```

Exact left/right placement is supported on X11. Wayland intentionally leaves window
placement to the desktop compositor.

The launcher detects the host light/dark preference. It can also be overridden:

```bash
MPREVIEW_THEME=dark ./mpreview.sh README.md
MPREVIEW_THEME=light ./mpreview.sh README.md
```

WebKit's inner process sandbox is disabled only for this nested-container workflow;
Docker networking and source mounts remain read-only. A normal host installation
does not need that environment variable.

## Usage

```bash
mpreview README.md
```

The preview responds to both direct writes and atomic file replacement, as commonly
used by terminal editors. Close the window to exit.
