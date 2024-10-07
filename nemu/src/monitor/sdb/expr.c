/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "common.h"
#include "debug.h"
#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <stdint.h>
#include <string.h>
#include <memory/vaddr.h>

enum {
  TK_NOTYPE = 256, TK_EQ, TK_AND, TK_NOT_EQ,

  /* TODO: Add more token types */
  TK_DECIMAL, TK_HEX, TK_REGS, TK_DEREF, TK_UNARY_MINUS
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
  {"!=", TK_NOT_EQ},        // not equal
  {"&&", TK_AND},        // AND
  {"\\$[0a-z]+", TK_REGS},   // registers
  {"0x[0-9a-f]+", TK_HEX},      // hex number
  {"[0-9]+", TK_DECIMAL},      // decimal integer
  {"-", '-'},             // minus
  {"/", '/'},             // division
  {"\\*", '*'},           // multiple
  {"\\(", '('},             // leftp
  {"\\)", ')'},             // rightp
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

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

static Token tokens[65536] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

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

        Log("%d: match rules[%d] = \"%s\" at position %d with len %d: %.*s, token type: %d",
            nr_token, i, rules[i].regex, position, substr_len, substr_len, substr_start, rules[i].token_type);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */
        switch (rules[i].token_type) {
          case TK_NOTYPE:  break;
          case TK_REGS:
          case TK_HEX:
          case TK_DECIMAL: Assert(substr_len < 32, "token str too long!");
                           tokens[nr_token].type = rules[i].token_type;
                           memcpy(tokens[nr_token].str, substr_start, substr_len);            \
                           tokens[nr_token].str[substr_len] = '\0';                                       \
                           nr_token++; break;
          case TK_EQ:
          case TK_NOT_EQ:
          case TK_AND:
          case '(':
          case ')':
          case '+':        
          case '-':
          case '*':        
          case '/':        tokens[nr_token].type = rules[i].token_type; nr_token++; break;
          default:         panic("unknown token type");
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

bool check_parentheses(int p, int q) {
  int lp = 0;

  for (int i = p; i <= q; i++) {
    if (tokens[i].type == '(') {
      lp++;
    } else if (tokens[i].type == ')') {
      lp--;
      if (lp < 0) {
        panic("left param and right param don't match!\n");
      } else if (lp == 0 && i != q) {
        return false;
      }
    }
  }

  return (tokens[p].type == '(' && tokens[q].type == ')') && lp == 0;
}

int find_position(int p, int q) {
  static const int N = 4;
  int rp = 0;
  int index[N];
  memset(index, 0xff, sizeof(int) * N);

  for (int i = q; i >= p; i--) {
    if ((tokens[i].type == TK_AND) && rp == 0 && index[0] == -1) {
      index[0] = i;
    } else if ((tokens[i].type == TK_EQ || tokens[i].type == TK_NOT_EQ) && rp == 0 && index[1] == -1) {
      index[1] = i;
    } else if (tokens[i].type == ')') {
      rp++;
    } else if (tokens[i].type == '(') {
      rp--;
    } else if ((tokens[i].type == '+' || tokens[i].type == '-') && rp == 0 && index[2] == -1) {
      index[2] = i;
    } else if ((tokens[i].type == '*' || tokens[i].type == '/') && rp == 0 && index[3] == -1) {
      index[3] = i;
    }
  }

  for (int i = 0; i < N; i++) {
    if (index[i] != -1) {
      return index[i];
    }
  }

  return -1;
}

word_t eval(int p, int q) {
  if (p > q) {
    /* Bad expression */
    panic("Bad expression\n");
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    if (tokens[p].type == TK_DECIMAL)
      return strtoull(tokens[p].str, NULL, 10);
    else if (tokens[p].type == TK_HEX)
      return strtoull(tokens[p].str, NULL, 16);
    else {
      // registers
      bool success;
      word_t ret = isa_reg_str2val(tokens[p].str + 1, &success);

      if (success) {
        return ret;
      } else {
        panic("register name %s not exists\n", tokens[p].str + 1);
      }
    }
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    return eval(p + 1, q - 1);
  }
  else {
    /* We should do more things here. */
    // Assert(tokens[p].type == TK_DECIMAL && tokens[q].type == TK_DECIMAL, "p and q should all be decimal ints!");
    // 4 +3*(2- 1)
    if (tokens[p].type == TK_DEREF) {
      word_t val2 = eval(p + 1, q);
      return vaddr_read(val2, sizeof(word_t));
    } else if (tokens[p].type == TK_UNARY_MINUS) {
      word_t val2 = eval(p + 1, q);
      return -((sword_t)val2);
    }
    int op = find_position(p, q);
    Assert(op > p, "no op between p and q");

    word_t val1 = eval(p, op - 1);
    word_t val2 = eval(op + 1, q);
    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': Assert(val2 != 0, "divide by zero!"); return val1 / val2;
      case TK_EQ: return val1 == val2;
      case TK_NOT_EQ: return val1 != val2;
      case TK_AND: return val1 && val2;
      default: Assert(0, "invalid op");
    }
  }
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  for (int i = 0; i < nr_token; i ++) {
    if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != TK_REGS && tokens[i - 1].type != TK_HEX \
          && tokens[i - 1].type != TK_DECIMAL && tokens[i - 1].type != ')')) ) {
      tokens[i].type = TK_DEREF;
    } else if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != TK_REGS && tokens[i - 1].type != TK_HEX \
          && tokens[i - 1].type != TK_DECIMAL && tokens[i - 1].type != ')')) ) {
      tokens[i].type = TK_UNARY_MINUS;
    }
  }
  *success = true;
  return eval(0, nr_token - 1);
}
