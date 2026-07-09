#!/bin/bash
# Thin wrapper: small in-guest compile+run smoke (same path as CI).
set -euo pipefail
cd "$(dirname "$0")/.."
LIMIT="${LIMIT:-8}"
RUN_TIMEOUT="${RUN_TIMEOUT:-5}"
export LIMIT RUN_TIMEOUT
exec ./scripts/run-qemu-repros.sh
