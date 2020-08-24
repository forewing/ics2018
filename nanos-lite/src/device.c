#include "common.h"
#include <amdev.h>
#include "device.h"
#include <klib.h>

int32_t SCREEN_H;
int32_t SCREEN_W;

size_t serial_write(const void *buf, size_t offset, size_t len) {
  int i;
  for (i = 0; i < len; i++){
    _putc(((const char*)buf)[i]);
  }
  return len;
}

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t offset, size_t len) {
  // printf("event read offset= %d len= %d\n", offset, len);
  char buf_local[128];
  int str_len;
  int key_ret = read_key();
  if ((key_ret & 0x7fff) != _KEY_NONE){
    str_len = sprintf(buf_local, "k%s %s\n", key_ret & 0x8000 ? "d" : "u", keyname[key_ret & 0x7fff]);
  }else{
    str_len = sprintf(buf_local, "t %d\n", uptime());
  }
  if (str_len > len){
    str_len = len;
  }
  memcpy(buf, buf_local, str_len);
  // prinstf(buf_local);
  return str_len;
}

static char dispinfo[128] __attribute__((used));

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  strncpy(buf, dispinfo + offset, len);
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len){
  int vga_ptr = offset / 4;
  void* buf_ptr = (void*)buf;
  while (buf_ptr < buf + len){
    int x = vga_ptr % SCREEN_W;
    int y = vga_ptr / SCREEN_W;
    int left = len + buf - buf_ptr;
    if (left / 4 >= SCREEN_W - x){
      draw_rect((void*)(buf), x, y, SCREEN_W - x, 1);
      vga_ptr += SCREEN_W - x;
      buf_ptr += (SCREEN_W - x) * 4;
    }else{
      draw_rect((void*)(buf), x, y, left / 4, 1);
      vga_ptr += left / 4;
      buf_ptr += left;
    }
  }
  return 0;
}

// size_t fb_write(const void *buf, size_t offset, size_t len) {
//   int vga_ptr = offset / 4;
//   int x = vga_ptr % SCREEN_W;
//   int y = vga_ptr / SCREEN_W;
//   int buf_ptr = 0;
//   while (buf_ptr < len){
//     x = vga_ptr % SCREEN_W;
//     y = vga_ptr / SCREEN_W;
//     if (x == 0){
//       draw_rect((uint32_t*)(buf + buf_ptr), x, y, SCREEN_W, 1);
//       vga_ptr += SCREEN_W;
//       buf_ptr += SCREEN_W*4;
//     }else{
//       draw_rect((uint32_t*)(buf + buf_ptr), x, y, 1, 1);
//       vga_ptr += 1;
//       buf_ptr += 4;
//     }
//   }
//   return 0;
// }

void init_device() {
  Log("Initializing devices...");
  _ioe_init();
  SCREEN_H = screen_height();
  SCREEN_W = screen_width();
  sprintf(dispinfo, "WIDTH:%d\nHEIGHT:%d", SCREEN_W, SCREEN_H);
  Log("WIDTH=%d HEIGHT=%d", SCREEN_W, SCREEN_H);
}
