#include <am.h>
#include <x86.h>
#include <amdev.h>

#define KBD_PORT 0x60



size_t input_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_INPUT_KBD: {
      uint32_t rd = inl(KBD_PORT);
      _KbdReg *kbd = (_KbdReg *)buf;
      kbd->keydown = 0;
      kbd->keycode = rd;
      return sizeof(_KbdReg);
    }
  }
  return 0;
}
