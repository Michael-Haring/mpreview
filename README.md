# mpreview

`mpreview` is a small native Linux Markdown previewer. It renders GitHub-Flavored
Markdown in a GTK window and reloads automatically when the source file is saved.

## Quick start

Build on the Linux desktop where you will run the preview. The preview uses that
host's GTK, WebKitGTK, and cmark-gfm libraries directly; Docker is not needed to
run it.

### 1. Install runtime dependencies

On Ubuntu 24.04, these packages must be installed on the desktop that runs
`mpreview`, even if you compile in Docker:

```bash
sudo apt update
sudo apt install libgtk-4-1 libwebkitgtk-6.0-4 libx11-6 \
  libcmark-gfm0.29.0.gfm.6 libcmark-gfm-extensions0.29.0.gfm.6
```

These are runtime libraries, not compiler packages. Apt installs their own
dependencies automatically.

### 2. Install build dependencies for a host build

`sudo make install` compiles on your host, so it also needs these packages:

```bash
sudo apt install build-essential cmake pkg-config git \
  libgtk-4-dev libwebkitgtk-6.0-dev libx11-dev \
  libcmark-gfm-dev libcmark-gfm-extensions-dev
```

Docker builds install their build dependencies inside the image. They are useful
for isolated compilation and tests; `sudo make install` builds on the host and
needs the host build packages above.

### 3. Clone, build, and install mpreview

```bash
git clone https://github.com/Michael-Haring/mpreview.git
cd mpreview
sudo make install
```

If you already cloned the repository, run `sudo make install` from it. This builds
and installs a small launcher at `/usr/local/bin/mpreview` and the native executable
at `/usr/local/lib/mpreview/mpreview`. The launcher checks for missing libraries
only if the executable fails to start; it never runs Docker.

### 4. Open a Markdown file

Run the preview as your normal user from the repository directory:

```bash
mpreview README.md
```

You can now run `mpreview /path/to/file.md` from any directory. Only installation
needs `sudo`. The application uses your desktop's theme and display directly.

## Development

```bash
make          # build a release executable for this host
make test     # build and run tests on this host
make run      # preview README.md from the repository
```

Docker remains available for isolated compilation and tests with `make image`,
`make docker-build`, and `make docker-test`. A binary built in the Ubuntu 22.04
image may need different library versions than those installed on your host, so
use the host build for installation.

Tile or maximize the preview at startup:

```bash
mpreview --right README.md
mpreview --left README.md
mpreview --maximized README.md
```

Exact left/right placement is supported on X11. Wayland intentionally leaves window
placement to the desktop compositor.

## Usage

```bash
mpreview README.md
```

The preview responds to both direct writes and atomic file replacement, as commonly
used by terminal editors. Close the window to exit.
