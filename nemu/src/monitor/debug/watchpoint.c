#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#include<stdlib.h>

#define NR_WP 128

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].last = 0;
    wp_pool[i].expr = NULL;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

WP* new_wp(){
  if (free_ == NULL){
    panic("WP poll full\n");
  }
  WP* tmp = free_;
  free_ = free_->next;
  tmp->next = head;
  head = tmp;
  if (tmp->next == NULL){
    tmp->NO = 0;
  }else{
    tmp->NO = tmp->next->NO + 1;
  }
  return tmp;
}

void free_wp(WP* wp){
  free(wp->expr);
  WP* tmp;
  if (wp == head){
    head = wp->next;
    wp->next = free_;    
    free_ = wp;
    return;
  }
  tmp = head;
  while(tmp->next != NULL){
    if (tmp->next == wp){
      tmp->next = wp->next;
      wp->next = free_;
      free_ = wp;
      return;
    }
    tmp = tmp->next;
  }
  printf("Watch point %p not found\n", wp);
  return;
}

WP* head_wp(){
  return head;
}
int chk_wp(){
  WP* tmp = head;
  bool succ = true;
  int val = 0;
  bool flag = 0;
  while (tmp != NULL){
    val = expr(tmp->expr, &succ);
    if (!succ){
      printf("Invalid expr at wp %d: %s\n", tmp->NO, tmp->expr);
      continue;
    }
    if (val != tmp->last){
      printf("WP %d: %s change to 0x%.8x\n", tmp->NO, tmp->expr, val);
      tmp->last = val;
      flag = 1;
    }
    tmp = tmp->next;
  }
  return flag;
}

/* TODO: Implement the functionality of watchpoint */


