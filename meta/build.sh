#!/usr/bin/env bash
set -euo pipefail

if [[ "$(uname)" == "Darwin" ]]; then
    HOST_OS="darwin"
else
    HOST_OS="linux"
fi
if [[ "${HOST:-}" == "" ]]; then
  HOST="$HOST_OS"
fi
if [[ "${TARGET:-}" == "" ]]; then
  TARGET="$HOST_OS"
fi

echo "HOST: $HOST"
echo "TARGET: $TARGET"

if command -v tup &> /dev/null; then
  echo "tup detected; using tup"

  ACTIVE_TUP_VERSION=$(tup -v)
  DESIRED_TUP_VERSION="tup v0.7.11-95-g4e9f5b32"

  if [[ "$ACTIVE_TUP_VERSION" != "$DESIRED_TUP_VERSION" ]]; then
    echo "Incorrect tup version; wanted $DESIRED_TUP_VERSION but got $ACTIVE_TUP_VERSION"
    exit 1
  fi

  ENV_BEFORE=$(env | sort)
  source "meta/envs/host/$HOST.env"
  source "meta/envs/target/$TARGET.env"
  ENV_AFTER=$(env | sort)

  echo ------------------------
  comm -13 <(echo "$ENV_BEFORE") <(echo "$ENV_AFTER")
  echo ------------------------

  DEFAULT_CMD=tup
else
  echo "tup not detected; using buildscripts"
  DEFAULT_CMD="meta/buildscripts/$TARGET-from-$HOST.sh"
fi

set -x

"${@:-$DEFAULT_CMD}"