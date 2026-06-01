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
# Builds and tests the big-endian SPARC port under QEMU user-mode.
#
# Intended to run inside the SPARC toolchain image (see
# tensorflow/lite/micro/testing/Dockerfile.sparc) on an x86-64 host, where the
# x86-64 SPARC cross toolchain runs natively and qemu-sparc/qemu-sparc64 runs
# at near-native (single-emulation) speed.
#
# On an arm64 host (e.g. Apple Silicon) the in-image qemu would be double
# emulated (Rosetta + TCG) and is impractically slow; use
# tensorflow/lite/micro/testing/sparc_docker_test.sh instead, which compiles in
# the amd64 image and runs the binaries in a native arm64 qemu container.
#
# Usage:
#   tensorflow/lite/micro/tools/ci_build/test_sparc.sh [TENSORFLOW_ROOT] [EXTERNAL_DIR]
#
# Environment:
#   SPARC_BITS  64 (default; SPARC V9 / sparc64) or 32 (SPARC V8). The provided
#               Docker image only ships a sparc64 toolchain, so 64 is the
#               default. For 32 supply a sparc-linux-gnu- toolchain on PATH.

set -e

TENSORFLOW_ROOT=${1}
EXTERNAL_DIR=${2}
source ${TENSORFLOW_ROOT}tensorflow/lite/micro/tools/ci_build/helper_functions.sh

export PATH=${PATH}:/opt/sparc64-toolchain/bin

TARGET=sparc_generic
SPARC_BITS=${SPARC_BITS:-64}
if [[ "${SPARC_BITS}" == "64" ]]; then
  TARGET_ARCH=sparc64
else
  TARGET_ARCH=sparc
fi

MAKEFILE=${TENSORFLOW_ROOT}tensorflow/lite/micro/tools/make/Makefile
COMMON_ARGS="TARGET=${TARGET} TARGET_ARCH=${TARGET_ARCH} SPARC_BITS=${SPARC_BITS} TENSORFLOW_ROOT=${TENSORFLOW_ROOT} EXTERNAL_DIR=${EXTERNAL_DIR}"

readable_run make -f ${MAKEFILE} ${COMMON_ARGS} config_info

readable_run make -f ${MAKEFILE} ${COMMON_ARGS} third_party_downloads

# Check that the release build is ok.
readable_run make -f ${MAKEFILE} clean TENSORFLOW_ROOT=${TENSORFLOW_ROOT} EXTERNAL_DIR=${EXTERNAL_DIR}
readable_run make $(get_parallel_jobs) -f ${MAKEFILE} ${COMMON_ARGS} BUILD_TYPE=release build

# Next build without release so failing tests give additional debug info, then
# run the full test suite under QEMU.
readable_run make -f ${MAKEFILE} clean TENSORFLOW_ROOT=${TENSORFLOW_ROOT} EXTERNAL_DIR=${EXTERNAL_DIR}
readable_run make $(get_parallel_jobs) -f ${MAKEFILE} ${COMMON_ARGS} build
readable_run make $(get_parallel_jobs) -f ${MAKEFILE} ${COMMON_ARGS} test
