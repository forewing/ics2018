#include <am.h>
#include <x86.h>
#include <amdev.h>
#include <klib.h>

#define SCREEN_PORT 0x100

#define VMEM 0x40000
#define VMEM_NR 0x80000

static uint32_t* const fb __attribute__((used)) = (uint32_t *)0x40000;

size_t video_read(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_INFO: {
      uint32_t rd = inl(SCREEN_PORT);
      _VideoInfoReg *info = (_VideoInfoReg *)buf;
      info->width = rd >> 16;
      info->height = rd & 0xffff;
      return sizeof(_VideoInfoReg);
    }
  }
  return 0;
}

size_t video_write(uintptr_t reg, void *buf, size_t size) {
  switch (reg) {
    case _DEVREG_VIDEO_FBCTL: {
      _FBCtlReg *ctl = (_FBCtlReg *)buf;
      int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
      uint32_t *pixels = ctl->pixels;
      int W = screen_width(), H = screen_height();
      int cp_bytes = sizeof(uint32_t) * (w < W-x ? w : W-x);
      int i;
      for (i = 0; i < h && y + i < H; i ++) {
        memcpy(&fb[(y + i) * W + x], pixels, cp_bytes);
        pixels += w;
      }
      // int i;
      // for (i = 0; i < screen_width() * screen_height(); i++){
      //   fb[i] = i;
      // }
      if (ctl->sync) {
        // do nothing, hardware syncs.
      }
      return sizeof(_FBCtlReg);
    }
  }
  return 0;
}

void vga_init() {
}
