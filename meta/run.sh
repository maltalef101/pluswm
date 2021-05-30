set -e

xinitrc_loc="$(git rev-parse --show-toplevel)/meta/xinitrc"

XEPHYR=$(whereis -b Xephyr | cut -f2 -d' ')
xinit $xinitrc_loc -- \
    "$XEPHYR" \
        :100 \
        -ac \
        -screen 800x600 \
        -host-cursor

