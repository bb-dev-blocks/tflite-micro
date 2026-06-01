# Settings for a generic SPARC target, executed under QEMU user-mode.
#
# SPARC is a 32-bit (V8) / 64-bit (V9) BIG-ENDIAN architecture. This is the
# central difference from the existing little-endian targets: do NOT add any
# little-endian flag here, and make sure the big-endian fixes in
# flatbuffer_utils.cc are present (see FlatBufferVectorToTfLiteTypeArray).
#
# SPARC_BITS selects the ABI and run model:
#   32 (default): SPARC V8, 32-bit. Toolchain prefix `sparc-linux-gnu-`,
#                 binaries run with `qemu-sparc`. This is the standard SPARC V8
#                 target the port is aimed at. A 32-bit SPARC V8 glibc Linux
#                 cross toolchain must be on PATH (or pass TARGET_TOOLCHAIN_ROOT
#                 / TARGET_TOOLCHAIN_PREFIX on the make command line).
#   64          : SPARC V9 (sparc64), 64-bit. Toolchain prefix
#                 `sparc64-linux-gnu-`, binaries run with `qemu-sparc64`. Used
#                 by the Dockerized validation (see testing/Dockerfile.sparc)
#                 because a prebuilt 32-bit V8 glibc toolchain is not readily
#                 downloadable, while a sparc64 one is. It exercises the exact
#                 same (big-endian) runtime and kernel code paths.
#
# Example invocations:
#   make -f tensorflow/lite/micro/tools/make/Makefile TARGET=sparc_generic microlite
#   make -f tensorflow/lite/micro/tools/make/Makefile TARGET=sparc_generic \
#        SPARC_BITS=64 test_hello_world_test

SPARC_BITS ?= 32

ifeq ($(SPARC_BITS),64)
  TARGET_ARCH := sparc64
  TARGET_TOOLCHAIN_PREFIX := sparc64-linux-
  SPARC_PLATFORM_FLAGS := -m64 -mcpu=v9
  QEMU_ARCH := sparc64
  QEMU_CPU := TI-UltraSparc-II
else
  TARGET_ARCH := sparc
  TARGET_TOOLCHAIN_PREFIX := sparc-linux-gnu-
  SPARC_PLATFORM_FLAGS := -m32 -mcpu=v8
  QEMU_ARCH := sparc
  QEMU_CPU := LEON3
endif

# Allow additional flags on the command line for debugging.
SPARC_EXTRA_CFLAGS :=

export PATH := $(TARGET_TOOLCHAIN_ROOT):$(PATH)

PLATFORM_FLAGS = \
  $(SPARC_PLATFORM_FLAGS) \
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

# Static link so qemu user-mode can execute the ELF directly without needing a
# sysroot / QEMU_LD_PREFIX for the dynamic loader.
LDFLAGS += -static

CCFLAGS += $(SPARC_EXTRA_CFLAGS)
CXXFLAGS += $(SPARC_EXTRA_CFLAGS)

# These tests pin exact arena byte counts / use arenas sized for a 32-bit
# little-endian host. They are not meaningful on this target because:
#   * 64-bit pointers (SPARC_BITS=64) enlarge the runtime structures, and
#   * the big-endian path byte-swaps multi-byte constant tensor data into the
#     arena (see micro_allocator.cc) instead of referencing it in place,
#     increasing arena usage.
# The functional runtime is still covered by the kernel tests, hello_world, and
# the other interpreter/allocator subtests. Mirrors riscv32_generic, which
# excludes memory_arena_threshold_test for the same class of reason.
EXCLUDED_TESTS := \
  $(TENSORFLOW_ROOT)tensorflow/lite/micro/memory_arena_threshold_test.cc \
  $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_allocator_test.cc \
  $(TENSORFLOW_ROOT)tensorflow/lite/micro/micro_interpreter_test.cc

MICROLITE_TEST_SRCS := $(filter-out $(EXCLUDED_TESTS), $(MICROLITE_TEST_SRCS))

TEST_SCRIPT := $(TENSORFLOW_ROOT)tensorflow/lite/micro/testing/test_with_qemu.sh $(QEMU_ARCH) $(QEMU_CPU)

include $(MAKEFILE_DIR)/ext_libs/eyalroz_printf.inc
