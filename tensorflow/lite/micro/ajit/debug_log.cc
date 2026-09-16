/* Copyright 2026 The TensorFlow Authors. All Rights Reserved.

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

// DebugLog for AJIT: write formatted text to UART MMIO (qemu-ajit map by default).

#include "tensorflow/lite/micro/debug_log.h"

#include <cstdint>

#ifndef TF_LITE_STRIP_ERROR_STRINGS
#include "eyalroz_printf/src/printf/printf.h"
#endif

#ifndef AJIT_UART_CONTROL
#define AJIT_UART_CONTROL 0xFFFF3200u
#endif
#ifndef AJIT_UART_TX
#define AJIT_UART_TX 0xFFFF3204u
#endif
#ifndef AJIT_UART_TX_BUSY
#define AJIT_UART_TX_BUSY 8u
#endif

namespace {

volatile uint32_t* UartControl() {
  return reinterpret_cast<volatile uint32_t*>(AJIT_UART_CONTROL);
}

volatile uint8_t* UartTx() {
  return reinterpret_cast<volatile uint8_t*>(AJIT_UART_TX);
}

void UartInit() {
  static bool ready = false;
  if (ready) {
    return;
  }
  *UartControl() = 3;
  ready = true;
}

void UartPutc(char c) {
  UartInit();
  while ((*UartControl() & AJIT_UART_TX_BUSY) != 0) {
  }
  *UartTx() = static_cast<uint8_t>(c);
}

void UartWrite(const char* s) {
  while (*s) {
    if (*s == '\n') {
      UartPutc('\r');
    }
    UartPutc(*s++);
  }
}

}  // namespace

extern "C" void DebugLog(const char* format, va_list args) {
#ifndef TF_LITE_STRIP_ERROR_STRINGS
  constexpr int kMaxLogLen = 256;
  char log_buffer[kMaxLogLen];
  vsnprintf_(log_buffer, kMaxLogLen, format, args);
  UartWrite(log_buffer);
#endif
}

#ifndef TF_LITE_STRIP_ERROR_STRINGS
extern "C" int DebugVsnprintf(char* buffer, size_t buf_size, const char* format,
                              va_list vlist) {
  return vsnprintf_(buffer, buf_size, format, vlist);
}
#endif
