#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  // printf("int: 0x%x\n", (int)NO);
  // printf("eip: 0x%x\n", (int)cpu.eip);
  // printf("idtr: 0x%x\n", (int)cpu.idtr);
  // printf("cpu.idtr:%x\n", cpu.idtr);

  rtl_push(&cpu.eflags);
  rtl_push(&cpu.cs);
  rtl_push(&ret_addr);
  rtl_li(&t0, cpu.idtr.base);
  rtl_host_lm(&t1, &NO, 1);
  rtl_mul_loi(&t1, &t1, 8);
  rtl_add(&t0, &t0, &t1);

  // printf("gate:%x\n", t0);
  if (t1 > cpu.idtr.limit){
    panic("Int NO too big!\n");
  }

  rtl_lm(&t1, &t0, 2);
  rtl_addi(&t0, &t0, 6);
  rtl_lm(&t0, &t0, 2);
  rtl_shli(&t0, &t0, 16);
  rtl_add(&t0, &t0, &t1);
  rtl_j(t0);
  // printf("NO:%d\naddr:%x\nidtr:%x", NO, t0, cpu.idtr.base);
}

void dev_raise_intr() {
}
