#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "$0")" && pwd)"
root_dir="$(cd "$script_dir/../.." && pwd)"
build_dir="$root_dir/build"

bundle_dir="$build_dir/examples/linux_x11_smoke_bundle"
mkdir -p "$bundle_dir/app"

if [ ! -f "$build_dir/linux_x11_smoke" ]; then
  echo "FAIL: linux_x11_smoke binary not found"
  exit 1
fi

cp "$build_dir/linux_x11_smoke" "$bundle_dir/"
strip "$bundle_dir/linux_x11_smoke"

cp "$root_dir/examples/linux_x11_smoke/app/index.html" "$bundle_dir/app/"
cp "$root_dir/examples/linux_x11_smoke/app/main.js" "$bundle_dir/app/"

actual_size=$(du -sb "$bundle_dir" | cut -f1)
max_size=10485760

echo "Bundle size: $actual_size bytes (limit: $max_size)"

if [ "$actual_size" -ge "$max_size" ]; then
  echo "FAIL: bundle exceeds 10 MB limit"
  exit 1
fi

echo "PASS: bundle under 10 MB"
