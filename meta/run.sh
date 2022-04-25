set -e

xinitrc_loc="$(git rev-parse --show-toplevel)/meta/xinitrc"

XNEST=$(whereis -b Xnest | cut -f2 -d' ')
xinit $xinitrc_loc -- \
    "$XNEST" \
        :100 \
        -ac \
