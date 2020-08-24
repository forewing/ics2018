#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */

#include "monitor/expr.h"
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

enum {
  TK_NOTYPE = 256,  // spaces

  TK_MULTI = 0,   // *
  TK_DIV = 1,     // /
  TK_PLUS = 2,    // +
  TK_MINUS = 3,   // -

  TK_L = 4, //<
  TK_G = 5, // >
  TK_LEQ = 6, // <=
  TK_GEQ = 7, // >=

  TK_EQ = 8,      // ==
  TK_NEQ = 9,

  TK_BIT_AND = 10,
  TK_BIT_XOR = 11,
  TK_BIT_OR = 12,

  TK_LOG_AND = 13, // &&
  TK_LOG_OR = 14,  // ||

  TK_BRAC_L = 130,  // (
  TK_BRAC_R = 131,  // )

  TK_NUM_BIN = 202, // 0b1111
  TK_NUM_OCT = 208, // 077777
  TK_NUM_DEC = 210, // 999999
  TK_NUM_HEX = 216, // 0xffff

  TK_REG = 230, // $eax
  TK_DEREF = 231, // *
  TK_NEG = 232 // -
  /* TODO: Add more token types */
};

static char OP_LEVEL[] = {
  0, 0, // * /
  1, 1, // + -
  2, 2, 2, 2, // < > <= >=
  3, 3, // == !=
  4, // &
  5, // ^
  6, // |
  7, // &&
  8 // ||
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {
  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */
  {" +", TK_NOTYPE},      //spaces

  {"\\*", TK_MULTI},      //multi
  {"/", TK_DIV},          //div
  {"\\+", TK_PLUS},       //plus
  {"-", TK_MINUS},        //minus

  {"<=", TK_LEQ},
  {">=", TK_GEQ},
  {"<", TK_L},
  {">", TK_G},

  {"==", TK_EQ},           //equal
  {"!=", TK_NEQ},           //not equal

  {"&&", TK_LOG_AND},           //logic and
  {"\\|\\|", TK_LOG_OR},           //logic or

  {"&", TK_BIT_AND},
  {"\\^", TK_BIT_XOR},
  {"\\|", TK_BIT_OR},

  {"\\(", TK_BRAC_L},     //left brac
  {"\\)", TK_BRAC_R},     //right brac

  {"0b[01]+", TK_NUM_BIN},        //bin number
  {"0x[0-9a-fA-F]+", TK_NUM_HEX}, //hex number
  {"[1-9][0-9]*|0", TK_NUM_DEC},    //dec number
  {"0[1-7][0-7]*", TK_NUM_OCT},   //oct number

  {"\\$[a-zA-z]+", TK_REG},   //reg name
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[65536];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        // Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
          case TK_NOTYPE:  //SPACE
            break;

          case TK_REG:
            tokens[nr_token].type = TK_REG;
            memcpy(tokens[nr_token].str, substr_start + 1, (substr_len - 1) * sizeof(char));
            tokens[nr_token].str[substr_len - 1] = '\0';
            nr_token++;
            break;

          case TK_NUM_BIN: //0b10101
          case TK_NUM_OCT: //077777
          case TK_NUM_DEC: //99999
          case TK_NUM_HEX: //0xFFFFF
            tokens[nr_token].type = rules[i].token_type;
            memcpy(tokens[nr_token].str, substr_start, (substr_len) * sizeof(char));
            tokens[nr_token].str[substr_len] = '\0';
            nr_token++;
            break;

          default:
            tokens[nr_token].type = rules[i].token_type;
            nr_token++;
            break;
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static int check_parentheses(int l, int r, bool* success){
  if (tokens[l].type != TK_BRAC_L || tokens[r].type != TK_BRAC_R){
    // Log("Left most %d and Right most %d are not bracs\n", l, r);
    return 0;
  }
  int ptr = 0, count = 1;
  int flag = 1;
  int *status = (int*)malloc(sizeof(int) * (r-l+1));
  int *stack = (int*)malloc(sizeof(int) * (r-l+1));
  for (int i = l; i <= r; i++){
    if (tokens[i].type == TK_BRAC_L){
      status[i-l] = count;
      stack[ptr] = count;
      count++;
      ptr++;
    }else if (tokens[i].type == TK_BRAC_R){
      if (ptr <= 0){
        *success = false;
        printf("Bad expr 234\n");
        flag = 0;
        break;
      }
      ptr--;
      status[i-l] = stack[ptr];
    }
  }
  if (ptr > 0){
    printf("Bad expr 243\n");
    *success = false;
    flag = 0;
  }
  if (status[0] != status[r-l]){
    flag = 0;
  }
  free(status);
  free(stack);
  return flag;
}

int cmpl_u2s(uint32_t src){
  int tmp;
  if (src > 0x7fffffff){
    src ^= 0xffffffff;
    src += 1;
    tmp = src;
    tmp *= -1;
  }else{
    tmp = src;
  }
  return tmp;
}

uint32_t cmpl_s2u(int src){
  uint32_t tmp;
  if (src >= 0){
    tmp = src;
  }else{
    src *= -1;
    tmp = src;
    tmp ^= 0xffffffff;
    tmp += 1;
  }
  return tmp;
}

static uint32_t str2int_any(int pos, bool* success){
  switch(tokens[pos].type){
    case TK_NUM_BIN:
      TODO(); // Bin2Dec
      break;
    case TK_NUM_OCT:
      TODO(); //Oct2Dec
      break;
    case TK_NUM_DEC:
      return str2int_dec(tokens[pos].str);
      break;
    case TK_NUM_HEX:
      return str2int_hex(tokens[pos].str);
      break;
    case TK_REG:
      return reg2int(tokens[pos].str);
      break;
    default:
      *success = false;
      return 0;
  }
}

static uint32_t eval(int l, int r, bool *success){
  if (tokens[l].type == TK_DEREF && r-l == 1){
    uint32_t tmp = str2int_any(r, success);
    if (success){
      return vaddr_read(tmp, 4);
    }
  }
  if (tokens[l].type == TK_NEG && r-l == 1){
    uint32_t tmp = str2int_any(r, success);
    if (success){
      return cmpl_s2u(-cmpl_u2s(tmp));
    }
  }
  if (l > r){
    Log("Bad expr 318\n");
    *success = false;
    return 0;
  }else if (l == r){
    return str2int_any(l, success);
  }else{
    int check = check_parentheses(l, r, success);
    if (!*success){
      return 0;
    }
    if (check){
      uint32_t tmp = eval(l+1, r-1, success);
      if (!*success){
        return 0;
      }
      return tmp;
    }else{
      int brac = 0;
      int oppos = -1;
      char level = 0;
      for (int i = l; i <= r; i++){
        switch(tokens[i].type){
          case TK_BRAC_L:
            brac++;
            break;
          case TK_BRAC_R:
            brac--;
            break;
          case TK_PLUS:
          case TK_MINUS:
          case TK_MULTI:
          case TK_DIV:
          case TK_LEQ:
          case TK_GEQ:
          case TK_L:
          case TK_G:
          case TK_EQ:
          case TK_NEQ:
          case TK_LOG_AND:
          case TK_LOG_OR:
          case TK_BIT_AND:
          case TK_BIT_XOR:
          case TK_BIT_OR:
            if (!brac){
              if (level <= OP_LEVEL[tokens[i].type]){
                level = OP_LEVEL[tokens[i].type];
                oppos = i;
              }
            }
            break;
          default:
            break;
        }
      }
      uint32_t val1 = eval(l, oppos-1, success);
      uint32_t val2 = eval(oppos+1, r, success);
      if (!*success){
        printf("Bad EXPR 378\n");
        return 0;
      }
      switch(tokens[oppos].type){
        case TK_PLUS:
          return cmpl_s2u(cmpl_u2s(val1) + cmpl_u2s(val2));
          // return val1 + val2;
          break;
        case TK_MINUS:
          return cmpl_s2u(cmpl_u2s(val1) - cmpl_u2s(val2));
          // return val1 - val2;
          break;
        case TK_MULTI:
          return cmpl_s2u(cmpl_u2s(val1) * cmpl_u2s(val2));
          // return val1 * val2;
          break;
        case TK_DIV:
          if (val2 == 0){
            *success = false;
            printf("ERROR: Div 0\n");
            return 0;
          }
          return cmpl_s2u(cmpl_u2s(val1) / cmpl_u2s(val2));
          // return val1 / val2;
          break;
        case TK_LEQ:
          return cmpl_u2s(val1) <= cmpl_u2s(val2);
          break;
        case TK_GEQ:
          return cmpl_u2s(val1) >= cmpl_u2s(val2);
          break;
        case TK_L:
          return cmpl_u2s(val1) < cmpl_u2s(val2);
          break;
        case TK_G:
          return cmpl_u2s(val1) > cmpl_u2s(val2);
          break;
        case TK_EQ:
          return val1 == val2;
          break;
        case TK_NEQ:
          return val1 != val2;
          break;
        case TK_LOG_AND:
          return (val1 != 0) && (val2 != 0);
          break;
        case TK_LOG_OR:
          return (val1 != 0) || (val2 != 0);
          break;
        case TK_BIT_AND:
          return val1 & val2;
          break;
        case TK_BIT_XOR:
          return val1 ^ val2;
          break;
        case TK_BIT_OR:
          return val1 | val2;
          break;
        default:
          *success = false;
          printf("ERROR: Unsupported op\n");
          return 0;
          break;
      }
    }
  }
  return 0;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  int i;
  for (i = 0; i < nr_token; i++){
    if (tokens[i].type == TK_MULTI){
      if (i != 0){
        if (tokens[i-1].type == TK_BRAC_R || tokens[i-1].type == TK_REG || tokens[i-1].type == TK_NUM_BIN || tokens[i-1].type == TK_NUM_DEC || tokens[i-1].type == TK_NUM_HEX){
          continue;
        }
      }
      printf("DEREF at %d\n", i);
      tokens[i].type = TK_DEREF;
    }
    if (tokens[i].type == TK_MINUS){
      if (i != 0){
        if (tokens[i-1].type == TK_BRAC_R || tokens[i-1].type == TK_REG || tokens[i-1].type == TK_NUM_BIN || tokens[i-1].type == TK_NUM_DEC || tokens[i-1].type == TK_NUM_HEX){
          continue;
        }
      }
      printf("NEG at %d\n", i);
      tokens[i].type = TK_NEG;
    }
  }
  return eval(0, nr_token-1, success);
}


int strlen_cst(char* str_head){
  int len = 0;

  for (len = 0; len < FW_STRING_MAXLEN; len++){
    if (*str_head == '\0'){
      return len;
    }else{
      str_head++;
    }
  }
  panic("String too long or invalid.");
  return len;
}

unsigned int str2int_dec(char* str_head){
  // int len = strlen_cst(str_head) - 1;
  unsigned int retval = 0;
  int last_digit;

  for (; *str_head != '\0'; str_head++){
    retval *= 10;
    last_digit = *str_head - '0';
    assert(last_digit <= 9 && last_digit >= 0);
    retval += last_digit;
  }

  return retval;
}

unsigned int str2int_hex(char* str_head){
  if (str_head[0] != '0' || !(str_head[1] == 'x' || str_head[1] == 'X')){
    panic("Invalid hex number");
  }

  unsigned int retval = 0;
  unsigned int last_digit;

  str_head += 2;
  for (; *str_head != '\0'; str_head++){
    retval *= 0x10;
    last_digit = *str_head;
    if (last_digit <= '9' && last_digit >= '0'){
      retval += last_digit - '0';
    }else if (last_digit <= 'f' && last_digit >= 'a'){
      retval += last_digit - 'a' + 10;
    }else if (last_digit <= 'F' && last_digit >= 'A'){
      retval += last_digit - 'A' + 10;
    }else{
      panic("Invalid hex number");
    }
  }

  return retval;
}

unsigned int reg2int(char* str_head){
  int i = 0;
  for (i = 0; i < 8; i++){
    if (strcmp(str_head, regsl[i]) == 0){
      return reg_l(i);
    }else if (strcmp(str_head, regsw[i]) == 0){
      return reg_w(i);
    }else if (strcmp(str_head, regsb[i]) == 0){
      return reg_b(i);
    }else if (strcmp(str_head, "eip") == 0){
      return cpu.eip;
    }
  }
  printf("Reg %s not found\n", str_head);
  return 0;
}