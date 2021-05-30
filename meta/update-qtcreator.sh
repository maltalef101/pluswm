#! /bin/sh

set -e

SRC_DIR="$(git rev-parse --show-toplevel)"

cd "$SRC_DIR"

find .  \( -name '*.cpp' -or -name '*.h' -or -name 'CMakeLists.txt' \) -print > pluswm.files
