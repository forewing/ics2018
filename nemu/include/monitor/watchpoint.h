#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  uint32_t last;
  char* expr;

} WP;

WP* new_wp();
void free_wp(WP* wp);
WP* head_wp();
int chk_wp();

#endif
