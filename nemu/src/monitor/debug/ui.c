#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "cpu/reg.h"
#include "nemu.h"
#include "debug.h"

// #include "libfw/FW_string.h"

#include <assert.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void monitor_statistic();

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_help(char *args);

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);
static int cmd_chkexpr(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si", "Step N steps, using: si [N]", cmd_si},
  { "info", "Get info of SUBCMD, using: info SUBCMD", cmd_info},
  { "x", "Scan N memory blocks of 4bytes from EXPR, using: x N EXPR", cmd_x},
  { "p", "Print the result of EXPR, using: p EXPR", cmd_p},
  { "w", "Stop when EXPR change, using: w EXPR", cmd_w},
  { "d", "Delete the N-th watch point, using: d N", cmd_d},
  { "chkexpr", "using:\nCorrectResult1 Expr1\nCorrectResult2 Expr2\nCorrectResult... Expr...", cmd_chkexpr}

  /* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args){
  char *arg = strtok(NULL, " ");
  unsigned int n;

  if (arg == NULL){
    cpu_exec(1);
  }else{
    n = str2int_dec(arg);
    // printf("Step %d steps\n", n);
    cpu_exec(n);
  }

  return 0;
}

static int cmd_info(char *args){
  char *arg = strtok(NULL, " ");
  if (arg == NULL){
    printf("ERROR: SUBCMD is required.\n");
    return 0;
  }

  if (strcmp(arg, "r") == 0){
    printf("Printing the info of regs:\n");
    printf("ID\tHex\tDec\n");

    int i = 0;
    for (i = R_EAX; i <= R_EDI; i++){
      printf("%s\t0x%.8x\t%d\n", regsl[i], reg_l(i), reg_l(i));
    }
    printf("%s\t0x%.8x\t%d\n", "eip", cpu.eip, cpu.eip);
    // for (i = R_AX; i <= R_DI; i++){
    //   printf("%s\t0x%.4hx\t%hd\n", regsw[i], reg_w(i), reg_w(i));
    // }
    // for (i = R_AL; i <= R_BH; i++){
    //   printf("%s\t0x%.2hhx\t%hhd\n", regsb[i], reg_b(i), reg_b(i));
    // }
  }else if (strcmp(arg, "w") == 0){
    printf("ID\tEXPR\tLast\n");
    WP* tmp = head_wp();
    while(tmp != NULL){
      printf("%d\t%s\t%d\n", tmp->NO, tmp->expr, tmp->last);
      tmp = tmp->next;
    }
  }else if (strcmp(arg, "f") == 0){
    printf("EFLAGS\n");
    printf("CF\t%d\n", cpu.CF);
    printf("OF\t%d\n", cpu.OF);
    printf("SF\t%d\n", cpu.SF);
    printf("ZF\t%d\n", cpu.ZF);
    printf("IF\t%d\n", cpu.IF);
  }else if (strcmp(arg, "s") == 0){
    monitor_statistic();
  }else{
    printf("Unsupported SUBCMD\n");
  }

  return 0;
}

static int cmd_x(char *args){
  char *arg1 = strtok(NULL, " ");
  if (arg1 == NULL){
    printf("ERROR: N is required.\n");
    return 0;
  }
  char *arg2 = strtok(NULL, "\n");
  if (arg2 == NULL){
    printf("ERROR: EXPR is required.\n");
    return 0;
  }

  bool succ = true;
  // unsigned int start = str2int_hex(arg2); //TODO: add full EXPR support
  unsigned int start = expr(arg2, &succ);
  if (!succ){
    printf("EXPR invalid.\n");
    return 0;
  }
  unsigned int end = start + str2int_dec(arg1) * 0x4;
  int counter = 0;

  for (; start < end; start += 0x4){
    if (counter % 4 == 0){
      printf("\n0x%.8x:\t", start);
    }
    printf("0x%.8x\t", vaddr_read(start, 0x4));
    counter++;
  }
  printf("\n");

  return 0;
}

static int cmd_p(char* args){
  char* arg = strtok(NULL, "\n");
  if (arg == NULL){
    printf("ERROR: EXPR is required.\n");
    return 0;
  }
  bool succ = true;
  unsigned int result = expr(arg, &succ);
  if (!succ){
    printf("EXPR invalid.\n");
    return 0;
  }
  printf("(signed)\t%s = %d\n", arg, cmpl_u2s(result));
  printf("(unsigned)\t%s = 0x%.8x\n", arg, result);
  return 0;
}

static int cmd_w(char *args){
  char* arg = strtok(NULL, "\n");
  if (arg == NULL){
    printf("ERROR: EXPR is required.\n");
    return 0;
  }
  bool succ = true;
  unsigned int result = expr(arg, &succ);
  if (!succ){
    printf("EXPR invalid.\n");
    return 0;
  }
  WP* wp = new_wp();
  wp->last = result;
  wp->expr = (char*)malloc(sizeof(char) * (strlen_cst(arg) + 1));
  memcpy(wp->expr, arg, sizeof(char) * (strlen_cst(arg) + 1));
  printf("Successfully set watch point \"%s\" with initial val %d\n", wp->expr, wp->last);
  return 0;
}

static int cmd_d(char *args){
  char* arg = strtok(NULL, " ");
  if (arg == NULL){
    printf("ERROR: EXPR is required.\n");
    return 0;
  }
  unsigned int no = str2int_dec(arg);
  WP* tmp = head_wp();
  while(tmp != NULL){
    if (tmp->NO == no){
      free_wp(tmp);
      return 0;
    }
    tmp = tmp->next;
  }
  return 0;
}

static int cmd_chkexpr(char *args){
  int i = 0;
  while(true){
    char* str = rl_gets();
    i++;
    char* num = strtok(str, " ");
    if (num == NULL){
      return 0;
    }
    char* sum = strtok(NULL, " \n");
    bool success = true;
    unsigned int numl = str2int_dec(num);
    unsigned int suml = expr(sum, &success);
    if (success){
      if (numl == suml){
        printf("T%d:\tEQ\n", i);
      }else{
        printf("T%d:\tNE\t%s\t%u\n", i, num, suml);
      }
    }
  }
  return 0;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

// TODO: make the following into expr
