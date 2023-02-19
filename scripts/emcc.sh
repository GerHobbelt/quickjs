#!/bin/bash
# Use system emcc if installed, otherwise use Docker.
set -eo pipefail

if [[ -z "$EMSDK_USE_DOCKER" ]] && command -v emcc >/dev/null; then
  if emcc --version | grep "$EMSDK_VERSION" >/dev/null; then
    exec emcc "$@"
  fi
fi

if [[ -z "$EMSDK_CACHE" ]]; then
  echo "EMSDK_CACHE must be set to a path"
fi

DOCKER_ARGV=(
  run --rm
  -v "$(pwd):$(pwd)"
  -u "$(id -u):$(id -g)"
  -w "$(pwd)"
)

if [[ -n "$EMSDK_DOCKER_CACHE" ]]; then
  # There are some files that are not pre-build in the Docker image,
  # and take a while to be automatically generated by emsdk.
  # We cache these on the host inside EMSDK_DOCKER_CACHE.
  EMSDK_DOCKER_CACHE_PATHS=(
    sysroot/lib/wasm32-emscripten/lto
    sysroot/lib/wasm32-emscripten/thinlto
    sysroot/lib/wasm32-emscripten/pic
    symbol_lists
  )
  for dir in "${EMSDK_DOCKER_CACHE_PATHS[@]}"; do
    host="$EMSDK_DOCKER_CACHE/$dir"
    guest="/emsdk/upstream/emscripten/cache/$dir"

    mkdir -p "$host"
    DOCKER_ARGV+=(
      -v "$host:$guest"
    )
  done
fi

DOCKER_ARGV+=(
  "${EMSDK_DOCKER_IMAGE}" emcc "$@"
)

set -x
exec docker "${DOCKER_ARGV[@]}"
