#!/usr/bin/env bash
#
# Build (if needed) the TFLM CI Docker image and run the hello_world example
# test inside it, mounting the local repo. Works natively on Apple Silicon
# (arm64) using the Make-based build, which does not require Bazel.
#
# Usage:
#   ./run_hello_world.sh
#
set -euo pipefail

IMAGE_NAME="tflm-ci"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

cd "${REPO_ROOT}"

if ! docker image inspect "${IMAGE_NAME}" >/dev/null 2>&1; then
  echo "Image '${IMAGE_NAME}' not found. Building from ci/Dockerfile.micro ..."
  docker build -f ci/Dockerfile.micro -t "${IMAGE_NAME}" .
else
  echo "Using existing image '${IMAGE_NAME}'."
fi

echo "Running hello_world_test inside the container ..."
docker run --rm \
  -v "${REPO_ROOT}":/opt/tflm \
  -w /opt/tflm \
  "${IMAGE_NAME}" \
  make -f tensorflow/lite/micro/tools/make/Makefile test_hello_world_test
