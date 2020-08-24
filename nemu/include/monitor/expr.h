#ifndef __EXPR_H__
#define __EXPR_H__

#include "common.h"

uint32_t expr(char *, bool *);

#define FW_STRING_MAXLEN 65536

int cmpl_u2s(uint32_t src);
uint32_t cmpl_s2u(int src);

int strlen_cst(char* str_head);
unsigned int str2int_dec(char* str_head);
unsigned int str2int_hex(char* str_head);
unsigned int reg2int(char* str_head);

#endif
