#! /bin/sh

set -e

SRC_DIR="$(git rev-parse --show-toplevel)"

cd "$SRC_DIR"

for i in $(find .  \( -name '*.cpp' -or -name '*.h' \)); do
	clang-format -i "$i"
done
