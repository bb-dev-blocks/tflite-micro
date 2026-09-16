# Copyright 2026 The TensorFlow Authors. All Rights Reserved.
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
# AJIT SPARC V8 32-bit (Buildroot sparc-linux- gcc). qemu-ajit / CoRTOS, not
# Linux qemu-user (that is TARGET=sparc_generic).
#
#   make -f tensorflow/lite/micro/tools/make/Makefile TARGET=ajit microlite
#
# UART DebugLog defaults to qemu-ajit TX 0xFFFF3204. C-model TX is 0xFFFF3210;
# rebuild DebugLog with -DAJIT_UART_TX=0xFFFF3210 -DAJIT_UART_CONTROL=0xFFFF3200
# when linking for the C simulator.

TARGET_ARCH := sparc
TARGET_TOOLCHAIN_PREFIX := sparc-linux-
# g++ binary is sparc-linux-g++ via ajit_env wrappers.
TARGET_TOOLCHAIN_ROOT :=

AJIT_PLATFORM_FLAGS := -m32 -mcpu=v8

PLATFORM_FLAGS = \
  $(AJIT_PLATFORM_FLAGS) \
  -fno-builtin-printf \
  -DTF_LITE_MCU_DEBUG_LOG \
  -DTF_LITE_USE_GLOBAL_CMATH_FUNCTIONS \
  -funsigned-char \
  -fno-delete-null-pointer-checks \
  -fomit-frame-pointer

CXXFLAGS += $(PLATFORM_FLAGS) \
  -fpermissive \
  -fno-use-cxa-atexit \
  -DTF_LITE_USE_GLOBAL_MIN \
  -DTF_LITE_USE_GLOBAL_MAX

CCFLAGS += $(PLATFORM_FLAGS)

LDFLAGS += -static

EXCLUDED_TESTS := \
  $(TENSORFLOW_ROOT)tensorflow/lite/micro/memory_arena_threshold_test.cc \
  $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_allocator_test.cc \
  $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_interpreter_test.cc

MICROLITE_TEST_SRCS := $(filter-out $(EXCLUDED_TESTS), $(MICROLITE_TEST_SRCS))

TARGET_SPECIFIC_MAKE_TEST := 1

include $(MAKEFILE_DIR)/ext_libs/eyalroz_printf.inc
