#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

enum{
  TYPE_NUM,
  TYPE_OP,
  TYPE_LB,
  TYPE_RB
};

#define MAX_DEPTH 20

// this should be enough
static char buf[65536] = "";
static inline void gen(int type){
  switch(type){
    case TYPE_NUM:{
      int dig = rand() % 9 + 1;
      char str[12] = "";
      int i;
      str[0] = rand() % 9 + '1';
      for (i = 1; i <= dig; i++){
        str[i] = rand() % 10 + '0';
      }
      str[dig+1] = '\0';
      strcat(buf, str);
      break;}
    case TYPE_OP:{
      int tmp = rand() % 4;
      switch(tmp){
        case 0:
          strcat(buf, "+");
          break;
        case 1:
          strcat(buf, "-");
          break;
        case 2:
          strcat(buf, "*");
          break;
        case 3:
          strcat(buf, "/");
          break;
        default:
          break;
      }
      break;}
    case TYPE_LB:
      strcat(buf, "(");
      break;
    default:
      strcat(buf, ")");
      break;
  }
  return;
}
static inline void gen_rand_expr(int depth) {
  if (depth > MAX_DEPTH){
    gen(TYPE_NUM);
    return;
  }
  switch(rand()%3){
    case 0:
      gen(TYPE_NUM);
      break;
    case 1:
      gen(TYPE_LB);
      gen_rand_expr(depth+1);
      gen(TYPE_RB);
      break;
    default:
      gen_rand_expr(depth+1);
      gen(TYPE_OP);
      gen_rand_expr(depth+1);
      break;
  }
  return;
}

static char code_buf[65536];
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  printf("chkexpr\n");
  for (i = 0; i < loop; i ++) {
    strcpy(buf, "");
    gen_rand_expr(0);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen(".code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc .code.c -o .expr");
    if (ret != 0) continue;

    fp = popen("./.expr", "r");
    assert(fp != NULL);

    int result;
    int fuckgcc = fscanf(fp, "%d", &result) == -1;
    if (fuckgcc < 0){
      printf("FUCK YOU GCC -Werror=unused-result");
    }
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}
