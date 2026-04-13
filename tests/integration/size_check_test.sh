#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "$0")" && pwd)"
root_dir="$(cd "$script_dir/../.." && pwd)"
build_dir="$root_dir/build/Debug"

bundle_dir="$build_dir/hello_viewshell_bundle"
mkdir -p "$bundle_dir/app"

if [ ! -f "$build_dir/hello_viewshell" ]; then
  echo "FAIL: hello_viewshell binary not found"
  exit 1
fi

cp "$build_dir/hello_viewshell" "$bundle_dir/"
strip "$bundle_dir/hello_viewshell"

cp "$root_dir/examples/hello_viewshell/app/index.html" "$bundle_dir/app/"
cp "$root_dir/examples/hello_viewshell/app/main.js" "$bundle_dir/app/"

actual_size=$(du -sb "$bundle_dir" | cut -f1)
max_size=10485760

echo "Bundle size: $actual_size bytes (limit: $max_size)"

if [ "$actual_size" -ge "$max_size" ]; then
  echo "FAIL: bundle exceeds 10 MB limit"
  exit 1
fi

echo "PASS: bundle under 10 MB"
