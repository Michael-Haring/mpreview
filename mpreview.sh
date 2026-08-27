#!/usr/bin/env bash

set -euo pipefail

readonly image_name="mpreview-dev"
readonly script_directory="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly install_prefix="$(cd -- "$script_directory/.." && pwd)"
readonly installed_executable="$install_prefix/lib/mpreview/mpreview"

if [[ -x $installed_executable ]]; then
    runtime_mount=(-v "$install_prefix/lib/mpreview:/opt/mpreview:ro")
    executable=/opt/mpreview/mpreview
else
    runtime_mount=(-v "$script_directory:/workspace:ro" -w /workspace)
    executable=./build-jammy/mpreview
fi

if [[ $# -eq 0 ]]; then
    echo "Usage: mpreview [--left|--right|--maximized] <file.md>" >&2
    exit 2
fi

if [[ $# -eq 1 && ( $1 == "--help" || $1 == "--version" ) ]]; then
    exec docker run --rm --network none \
        --user "$(id -u):$(id -g)" \
        -e HOME=/tmp \
        "${runtime_mount[@]}" \
        "$image_name" "$executable" "$1"
fi

placement_options=()
document_argument=
for argument in "$@"; do
    case $argument in
        --left|--right|--maximized)
            placement_options+=("$argument")
            ;;
        --*)
            echo "mpreview: unknown option: $argument" >&2
            exit 2
            ;;
        *)
            if [[ -n $document_argument ]]; then
                echo "mpreview: expected one Markdown file" >&2
                exit 2
            fi
            document_argument=$argument
            ;;
    esac
done

if [[ -z $document_argument ]]; then
    echo "Usage: mpreview [--left|--right|--maximized] <file.md>" >&2
    exit 2
fi

if [[ ! -f $document_argument ]]; then
    echo "mpreview: file does not exist: $document_argument" >&2
    exit 1
fi

readonly document_path="$(realpath -- "$document_argument")"
readonly document_directory="$(dirname -- "$document_path")"
readonly document_name="$(basename -- "$document_path")"

theme=${MPREVIEW_THEME:-system}
if [[ $theme == system ]]; then
    color_scheme="$(gsettings get org.gnome.desktop.interface color-scheme 2>/dev/null || true)"
    gtk_theme="$(gsettings get org.gnome.desktop.interface gtk-theme 2>/dev/null || true)"
    if [[ $color_scheme == *prefer-dark* || $gtk_theme == *[Dd]ark* ]]; then
        theme=dark
    else
        theme=light
    fi
fi

case $theme in
    dark)
        readonly container_gtk_theme="Adwaita:dark"
        ;;
    light)
        readonly container_gtk_theme="Adwaita"
        ;;
    *)
        echo "mpreview: MPREVIEW_THEME must be 'system', 'dark', or 'light'" >&2
        exit 2
        ;;
esac

xauthority=${XAUTHORITY:-}
if [[ -z $xauthority && -n ${XDG_RUNTIME_DIR:-} && -f $XDG_RUNTIME_DIR/gdm/Xauthority ]]; then
    xauthority=$XDG_RUNTIME_DIR/gdm/Xauthority
elif [[ -z $xauthority && -f $HOME/.Xauthority ]]; then
    xauthority=$HOME/.Xauthority
fi

if [[ -z $xauthority || ! -f $xauthority ]]; then
    echo "mpreview: unable to locate the X11 authority file" >&2
    exit 1
fi

exec docker run --rm --network none \
    --user "$(id -u):$(id -g)" \
    -e HOME=/tmp \
    -e DISPLAY \
    -e XAUTHORITY=/tmp/mpreview.xauth \
    -e NO_AT_BRIDGE=1 \
    -e GTK_A11Y=none \
    -e GTK_THEME="$container_gtk_theme" \
    -e LIBGL_ALWAYS_SOFTWARE=1 \
    -e WEBKIT_DISABLE_SANDBOX_THIS_IS_DANGEROUS=1 \
    -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
    -v "$xauthority:/tmp/mpreview.xauth:ro" \
    -v "$document_directory:/document:ro" \
    "${runtime_mount[@]}" \
    "$image_name" "$executable" "${placement_options[@]}" \
    "/document/$document_name" \
    2> >(grep -v "Can't connect to a11y bus" >&2)
