# mpreview

`mpreview` is a small native Linux Markdown previewer. It renders GitHub-Flavored
Markdown in a GTK window and reloads automatically when the source file is saved.

## Quick start

The host only needs Docker Engine, `make`, and an X11 display (native X11 or XWayland).
GTK, WebKitGTK, cmark-gfm, CMake, and the compiler stay inside the Docker image.

From the cloned repository:

```bash
make image
sudo make install
mpreview README.md
```

Your user must be able to run Docker without `sudo`. If Docker reports permission
denied, add the user to the Docker group and then log out and back in:

```bash
sudo usermod -aG docker "$USER"
```

To refresh the group in the current terminal instead:

```bash
newgrp docker
```

## Development

```bash
make          # build in Docker
make test     # build and run tests in Docker
make run      # preview README.md from the repository
```

Tile or maximize the preview at startup:

```bash
mpreview --right README.md
mpreview --left README.md
mpreview --maximized README.md
```

Exact left/right placement is supported on X11. Wayland intentionally leaves window
placement to the desktop compositor.

The launcher detects the host light/dark preference. It can also be overridden:

```bash
MPREVIEW_THEME=dark mpreview README.md
MPREVIEW_THEME=light mpreview README.md
```

Docker networking and source mounts are read-only at runtime.

## Usage

```bash
mpreview README.md
```

The preview responds to both direct writes and atomic file replacement, as commonly
used by terminal editors. Close the window to exit.
