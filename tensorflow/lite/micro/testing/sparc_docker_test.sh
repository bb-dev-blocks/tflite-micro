#!/usr/bin/env bash
# Copyright 2024 The TensorFlow Authors. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ==============================================================================
#
# End-to-end big-endian SPARC build+run, fully inside Docker, that works on both
# x86-64 and Apple Silicon hosts.
#
# Why two images: the prebuilt SPARC cross toolchain is x86-64, so it must run
# in an amd64 container (native on x86-64, Rosetta on Apple Silicon). qemu
# inside that amd64 container would be double-emulated on Apple Silicon and is
# unusably slow, so the compiled SPARC ELF binaries are instead executed in a
# separate NATIVE-architecture runner image where qemu-sparc64 emulates SPARC
# directly. On a native x86-64 host both images run natively.
#
# Usage:
#   tensorflow/lite/micro/testing/sparc_docker_test.sh [target ...]
# Examples:
#   tensorflow/lite/micro/testing/sparc_docker_test.sh                # hello_world_test
#   tensorflow/lite/micro/testing/sparc_docker_test.sh hello_world_test kernel_conv_test

set -euo pipefail

REPO_ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/../../../.." && pwd)
cd "${REPO_ROOT}"

BUILD_IMAGE=${BUILD_IMAGE:-tflm-sparc}
RUNNER_IMAGE=${RUNNER_IMAGE:-tflm-sparc-runner}
SPARC_BITS=${SPARC_BITS:-64}
TARGET_ARCH=sparc64
PASS_STRING='~~~ALL TESTS PASSED~~~'

TARGETS=("$@")
if [[ ${#TARGETS[@]} -eq 0 ]]; then
  TARGETS=(hello_world_test)
fi

MAKEFILE=tensorflow/lite/micro/tools/make/Makefile
MAKE_ARGS="TARGET=sparc_generic TARGET_ARCH=${TARGET_ARCH} SPARC_BITS=${SPARC_BITS}"

echo "==> Ensuring images exist"
if ! docker image inspect "${BUILD_IMAGE}" >/dev/null 2>&1; then
  docker build --platform=linux/amd64 \
    -f tensorflow/lite/micro/testing/Dockerfile.sparc -t "${BUILD_IMAGE}" .
fi
if ! docker image inspect "${RUNNER_IMAGE}" >/dev/null 2>&1; then
  docker build \
    -f tensorflow/lite/micro/testing/Dockerfile.sparc-runner -t "${RUNNER_IMAGE}" .
fi

echo "==> Compiling SPARC binaries in ${BUILD_IMAGE} (amd64): ${TARGETS[*]}"
docker run --rm --platform=linux/amd64 -v "${REPO_ROOT}":/opt/tflm -w /opt/tflm \
  "${BUILD_IMAGE}" bash -c "
    export PATH=\$PATH:/opt/sparc64-toolchain/bin
    make -f ${MAKEFILE} ${MAKE_ARGS} third_party_downloads >/dev/null
    make -j\$(nproc) -f ${MAKEFILE} ${MAKE_ARGS} ${TARGETS[*]}
  "

BINDIR=$(ls -d gen/sparc_generic_${TARGET_ARCH}_*/bin)
echo "==> Running SPARC binaries under native qemu-sparc64 in ${RUNNER_IMAGE}"
docker run --rm -v "${REPO_ROOT}":/opt/tflm -w /opt/tflm "${RUNNER_IMAGE}" bash -c "
  fail=0
  for t in ${TARGETS[*]}; do
    out=\$(timeout 300 qemu-sparc64 \"${BINDIR}/\$t\" 2>&1) || true
    if echo \"\$out\" | grep -q '${PASS_STRING}'; then
      echo \"PASS  \$t\"
    else
      echo \"FAIL  \$t\"
      echo \"\$out\" | tail -8
      fail=1
    fi
  done
  echo \"==> OVERALL: \$([ \$fail -eq 0 ] && echo ALL_PASS || echo SOME_FAILED)\"
  exit \$fail
"
