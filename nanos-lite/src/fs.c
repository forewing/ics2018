#include "common.h"
#include "fs.h"
#include "ramdisk.h"
#include "device.h"

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_TTY, FD_FB, FD_EVENT};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin", 0, 0, invalid_read, invalid_write},
  {"stdout", 0, 0, invalid_read, serial_write},
  {"stderr", 0, 0, invalid_read, serial_write},
  {"/dev/tty", 128, 0, invalid_read, serial_write},
  {"/dev/fb", 0, 0, invalid_read, fb_write},
  {"/dev/events", 65535, 0, events_read, invalid_write},
  {"/proc/dispinfo", 128, 0, dispinfo_read, invalid_write},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

extern int32_t SCREEN_H;
extern int32_t SCREEN_W;

void init_fs() {
  // TODO: initialize the size of /dev/fb
  file_table[FD_FB].size = SCREEN_H * SCREEN_W * sizeof(uint32_t);
  file_table[FD_FB].open_offset = 0;
  Log("init_fs fb.size=%d", file_table[FD_FB].size);
}

int fs_open(const char *pathname, int flags, int mode){
  int i;
  for (i = FD_STDERR + 1; i < NR_FILES; i++){
    if (strcmp(file_table[i].name, pathname) == 0){
      file_table[i].open_offset = 0;
      return i;
    }
  }
  panic("File not found: %s\n", pathname);
  return -1;
}

size_t fs_read(int fd, void *buf, size_t len){
  if (fd == FD_EVENT){
    return file_table[fd].read(buf, 0, len);
  }
  size_t ptr_end = file_table[fd].open_offset + len;
  if (ptr_end > file_table[fd].size){
    len = file_table[fd].size - file_table[fd].open_offset;
    ptr_end = file_table[fd].size;
  }
  size_t ptr_head = file_table[fd].disk_offset + file_table[fd].open_offset;
  if (file_table[fd].read != NULL){
    file_table[fd].read(buf, ptr_head, len);
  }else{
    ramdisk_read(buf, ptr_head, len);
  }
  file_table[fd].open_offset = ptr_end;
  return len;
};

size_t fs_write(int fd, const void *buf, size_t len){
  if (fd == FD_STDOUT || fd == FD_STDERR || fd == FD_STDIN || fd == FD_TTY){
    return serial_write(buf, 0, len);
  }
  size_t ptr_end = file_table[fd].open_offset + len;
  if (ptr_end > file_table[fd].size){
    len = file_table[fd].size - file_table[fd].open_offset;
    ptr_end = file_table[fd].size;
  }
  size_t ptr_head = file_table[fd].disk_offset + file_table[fd].open_offset;
  if (file_table[fd].write != NULL){
    file_table[fd].write(buf, ptr_head, len);
  }else{
    ramdisk_write(buf, ptr_head, len);
  }
  file_table[fd].open_offset = ptr_end;
  return len;
}

size_t fs_lseek(int fd, size_t offset, int whence){
  size_t target = offset;
  switch(whence){
    case SEEK_SET:
      break;
    case SEEK_CUR:
      target += file_table[fd].open_offset;
      break;
    case SEEK_END:
      target = file_table[fd].size;
      break;
    default:
      break;
  }
  if (target > file_table[fd].size){
    Log("Set too more");
    return target;
  }
  file_table[fd].open_offset = target;
  return target;
}

int fs_close(int fd){
  return 0;
}

size_t fs_filesz(int fd){
  return file_table[fd].size;
}