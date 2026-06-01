/* Copyright 2024 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

// Debug logging for the generic SPARC target. When running under QEMU
// user-mode (qemu-sparc / qemu-sparc64) the host services the write() syscall,
// so writing to stdout is enough to surface logs (including the test pass
// string) to the test runner. This mirrors the riscv32_generic target.

#include "tensorflow/lite/micro/debug_log.h"

#include <cstdio>

#ifndef TF_LITE_STRIP_ERROR_STRINGS
#include "eyalroz_printf/src/printf/printf.h"
#endif

extern "C" void DebugLog(const char* format, va_list args) {
#ifndef TF_LITE_STRIP_ERROR_STRINGS
  constexpr int kMaxLogLen = 256;
  char log_buffer[kMaxLogLen];

  vsnprintf_(log_buffer, kMaxLogLen, format, args);
  std::fputs(log_buffer, stdout);
  std::fflush(stdout);
#endif
}

#ifndef TF_LITE_STRIP_ERROR_STRINGS
// Only called from MicroVsnprintf (micro_log.h)
extern "C" int DebugVsnprintf(char* buffer, size_t buf_size, const char* format,
                              va_list vlist) {
  return vsnprintf_(buffer, buf_size, format, vlist);
}
#endif
