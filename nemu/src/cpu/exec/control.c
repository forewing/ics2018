#include "cpu/exec.h"
#include "cpu/cc.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  rtl_j(decoding.jmp_eip);

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint32_t cc = decoding.opcode & 0xf;
  rtl_setcc(&t0, cc);
  rtl_li(&t1, 0);
  rtl_jrelop(RELOP_NE, &t0, &t1, decoding.jmp_eip);
  print_asm("j%s %x", get_cc_name(cc), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  rtl_jr(&id_dest->val);

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  if (decoding.is_operand_size_16){
    TODO();
  }else{
    // printf("!!eip: %.8x seq: %.8x jmp: %.8x\n", cpu.eip, decoding.seq_eip, decoding.jmp_eip);
    // if (get_steps() > 2435633){
    //   Log("call esp: %x ret_addr: %x jmp_addr: %x", cpu.esp-4, decoding.seq_eip, decoding.jmp_eip);
    // }
    rtl_push(&decoding.seq_eip);
    rtl_j(decoding.jmp_eip);
  }

  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  // TODO();
  rtl_pop(&t0);
  // rtl_host_sm(&decoding.jmp_eip, &t0, 4);
  // rtl_pop(&decoding.seq_eip);
  // if (get_steps() > 2435633){
  //   Log(" ret  esp: %x ret_addr: %x eip:%x", cpu.esp-4, t0, cpu.eip);
  // }
  rtl_j(t0);
  // printf("!!eip: %.8x seq: %.8x jmp: %.8x\n", cpu.eip, decoding.seq_eip, decoding.jmp_eip);
  print_asm("ret");
}

make_EHelper(call_rm) {
  rtl_push(&decoding.seq_eip);
  rtl_j(id_dest->val);

  print_asm("call *%s", id_dest->str);
}
