#include "common.h"
#include "syscall.h"
#include "fs.h"
#include "loader.h"

int do_syscall_brk(uint32_t increment){
  static intptr_t _end_log = 0;
  if (_end_log == 0){
    _end_log = (intptr_t)_heap.start;
  }
  intptr_t ret = _end_log;
  _end_log += increment;
  return ret;
}

int do_syscall_execve(const char *fname, void* argv, void* envp){
  printf("Hit execve\n");
  naive_uload(NULL, fname);
  return -1;
}

_Context* do_syscall(_Context *c) {
  // _putc('!');
  uintptr_t a[4];
  a[0] = c->GPR1;
  a[1] = c->GPR2;
  a[2] = c->GPR3;
  a[3] = c->GPR4;
  switch (a[0]) {
    case SYS_exit:  do_syscall_execve("/bin/init", 0, 0);
      break;
    case SYS_yield: _yield(); c->GPRx = 0;
      break;
    case SYS_open:  c->GPRx = fs_open((const char*)(a[1]), a[2], a[3]);
      break;
    case SYS_read:  c->GPRx = fs_read(a[1], (char*)(a[2]), a[3]);
      break;
    case SYS_write: c->GPRx = fs_write(a[1], (void*)a[2], a[3]);
      break;
    case SYS_close: c->GPRx = fs_close(a[1]);
      break;
    case SYS_lseek: c->GPRx = fs_lseek(a[1], a[2], a[3]);
      break;
    case SYS_brk:   c->GPRx = do_syscall_brk(a[1]);
      break;
    case SYS_fstat: panic("fstat not finished\n");
      break;
    case SYS_execve: c->GPRx = do_syscall_execve((const char*)(a[1]), (void*)(a[2]), (void*)(a[3]));
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
