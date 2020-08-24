#include "cpu/exec.h"

void difftest_skip_ref();
void difftest_skip_dut();

uint32_t pio_read_l(ioaddr_t addr);
uint32_t pio_read_w(ioaddr_t addr);
uint32_t pio_read_b(ioaddr_t addr);
void pio_write_l(ioaddr_t addr, uint32_t data);
void pio_write_w(ioaddr_t addr, uint32_t data);
void pio_write_b(ioaddr_t addr, uint32_t data);

void raise_intr(uint8_t NO, vaddr_t ret_addr);

make_EHelper(lidt) {
  if (decoding.is_operand_size_16){
    TODO();
  }else{
    rtl_lm((uint32_t*)&cpu.idtr.limit, &id_dest->addr, 2);
    rtl_addi(&t0, &id_dest->addr, 2);
    rtl_lm(&cpu.idtr.base, &t0, 4);
  }
  // printf("LIDT limit:%x base:%x\n", cpu.idtr.limit, cpu.idtr.base);
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(int) {
  raise_intr(id_dest->val, decoding.seq_eip);
  print_asm("int %s", id_dest->str);

#if defined(DIFF_TEST) && defined(DIFF_TEST_QEMU)
  difftest_skip_dut();
#endif
}

make_EHelper(iret) {
  if (decoding.is_operand_size_16){
    TODO();
  }else{
    rtl_pop(&t0);
    rtl_j(t0);
    rtl_pop(&cpu.cs);
    rtl_pop(&cpu.eflags);
  }

  print_asm("iret");
}

make_EHelper(in) {
  rtl_host_lm(&t0, &id_src->val, id_src->width);
  switch(id_dest->width){
    case 4:
      t1 = pio_read_l(t0);
      break;
    case 2:
      t1 = pio_read_w(t0);
      break;
    case 1:
      t1 = pio_read_b(t0);
      break;
  }
  operand_write(id_dest, &t1);

  print_asm_template2(in);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}

make_EHelper(out) {
  rtl_host_lm(&t0, &id_dest->val, id_dest->width);
  rtl_host_lm(&t1, &id_src->val, id_src->width);

  switch(id_src->width){
    case 4:
      pio_write_l(t0, t1);
      break;
    case 2:
      pio_write_w(t0, t1);
      break;
    case 1:
      pio_write_b(t0, t1);
      break;
  }

  print_asm_template2(out);

#if defined(DIFF_TEST)
  difftest_skip_ref();
#endif
}
