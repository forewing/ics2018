#include <dlfcn.h>

#include "nemu.h"
#include "monitor/monitor.h"
#include "diff-test.h"

int QEMU_SKIP_FLAGS;

static void (*ref_difftest_memcpy_from_dut)(paddr_t dest, void *src, size_t n);
static void (*ref_difftest_getregs)(void *c);
static void (*ref_difftest_setregs)(const void *c);
static void (*ref_difftest_exec)(uint64_t n);

static bool is_skip_ref;
static bool is_skip_dut;

void difftest_skip_ref() { is_skip_ref = true; }
void difftest_skip_dut() { is_skip_dut = true; }

void init_difftest(char *ref_so_file, long img_size) {
#ifndef DIFF_TEST
  return;
#endif

  QEMU_SKIP_FLAGS = 0;

  assert(ref_so_file != NULL);

  void *handle;
  handle = dlopen(ref_so_file, RTLD_LAZY | RTLD_DEEPBIND);
  assert(handle);

  ref_difftest_memcpy_from_dut = dlsym(handle, "difftest_memcpy_from_dut");
  assert(ref_difftest_memcpy_from_dut);

  ref_difftest_getregs = dlsym(handle, "difftest_getregs");
  assert(ref_difftest_getregs);

  ref_difftest_setregs = dlsym(handle, "difftest_setregs");
  assert(ref_difftest_setregs);

  ref_difftest_exec = dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

  void (*ref_difftest_init)(void) = dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  Log("Differential testing: \33[1;32m%s\33[0m", "ON");
  Log("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in include/common.h.", ref_so_file);

  ref_difftest_init();
  ref_difftest_memcpy_from_dut(ENTRY_START, guest_to_host(ENTRY_START), img_size);
  ref_difftest_setregs(&cpu);
}

#define SET_STATUS if (nemu_state != NEMU_ABORT){ \
    monitor_statistic(); \
    printf("name\tnemu\tqemu\n"); \
    nemu_state = NEMU_ABORT; \
  }

void difftest_step(uint32_t eip) {
  CPU_state ref_r;

  if (is_skip_dut) {
    is_skip_dut = false;
    return;
  }

  if (is_skip_ref) {
    // to skip the checking of an instruction, just copy the reg state to reference design
    ref_difftest_setregs(&cpu);
    is_skip_ref = false;
    return;
  }

  ref_difftest_exec(1);
  ref_difftest_getregs(&ref_r);

  // TODO: Check the registers state with the reference design.
  // Set `nemu_state` to `NEMU_ABORT` if they are not the same.
  // int i = 0;
  // for (i = R_EAX; i < R_EDI; i++){
  //   if(cpu.gpr[i]._32 != ref_r.gpr[i]._32){
  //     SET_STATUS;
  //     printf("%s\t0x%.8x\t0x%.8x\n", reg_name(i, 4), cpu.gpr[i]._32, ref_r.gpr[i]._32);
  //   }
  // }
  if(cpu.eip != ref_r.eip){
    SET_STATUS;
    printf("EIP\t0x%.8x\t0x%.8x\n", cpu.eip, ref_r.eip);
  }
  // if (!QEMU_SKIP_FLAGS){
  //   if (cpu.ZF != ref_r.ZF || cpu.SF != ref_r.SF){
  //     SET_STATUS;
  //     printf("ZF\t%d\t%d\n", cpu.ZF, ref_r.ZF);
  //     printf("SF\t%d\t%d\n", cpu.SF, ref_r.SF);
  //   }
  // }
  QEMU_SKIP_FLAGS = 0;
  // printf("CF\t%d\t%d\n", cpu.CF, ref_r.CF);
  // printf("OF\t%d\t%d\n", cpu.OF, ref_r.OF);
}
