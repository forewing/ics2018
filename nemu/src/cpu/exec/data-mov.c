#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  rtl_push(&id_dest->val);
  print_asm_template1(push);
}

make_EHelper(pop) {
  rtl_pop(&t0);
  operand_write(id_dest, &t0);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  if (decoding.is_operand_size_16){
    TODO();
  }else{
    rtl_host_lm(&t0, &cpu.esp, 4);
    rtl_push(&cpu.eax);
    rtl_push(&cpu.ecx);
    rtl_push(&cpu.edx);
    rtl_push(&cpu.ebx);
    rtl_push(&t0);
    rtl_push(&cpu.ebp);
    rtl_push(&cpu.esi);
    rtl_push(&cpu.edi);
  }

  print_asm("pusha");
}

make_EHelper(popa) {
  if (decoding.is_operand_size_16){
    TODO();
  }else{
    rtl_pop(&cpu.edi);
    rtl_pop(&cpu.esi);
    rtl_pop(&cpu.ebp);
    rtl_pop(&cpu.ebx); // throw away
    rtl_pop(&cpu.ebx);
    rtl_pop(&cpu.edx);
    rtl_pop(&cpu.ecx);
    rtl_pop(&cpu.eax);
  }
  print_asm("popa");
}

make_EHelper(leave) {
  rtl_lr(&t0, R_EBP, 4);
  rtl_sr(R_ESP, &t0, 4);
  if (decoding.is_operand_size_16){
    TODO();
  }else{
    rtl_pop(&t0);
    rtl_sr(R_EBP, &t0, 4);
  }
  print_asm("leave");
}

make_EHelper(cltd) {
  if (decoding.is_operand_size_16) {
    TODO();
  }
  else {
    TODO();
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    TODO();
  }
  else {
    TODO();
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t0, &id_src->val, id_src->width);
  operand_write(id_dest, &t0);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  // TODO();
  operand_write(id_dest, &id_src->addr);
  print_asm_template2(lea);
}

make_EHelper(xchg){
  rtl_host_lm(&t0, &id_dest->val, id_dest->width);
  rtl_host_lm(&t1, &id_src->val, id_src->width);
  operand_write(id_dest, &t1);
  operand_write(id_src, &t0);
  print_asm_template2(xchg);
}

make_EHelper(cbw){
  rtl_sext(&t0, &id_src->val, id_dest->width);
  operand_write(id_dest, &t0);
  print_asm_template2(cbw);
}

make_EHelper(cwd){
  rtl_msb(&t0, &id_src->val, id_src->width);
  switch(id_dest->width){
    case 2:
      t1 = 0xffff;
      break;
    case 4:
      t1 = 0xffffffff;
      break;
  }
  rtl_mul_lo(&t0, &t0, &t1);
  operand_write(id_dest, &t0);
  print_asm_template2(cwd);
}