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
#include <string.h>

enum {
  TK_NOTYPE = 256, TK_EQ,

  /* TODO: Add more token types */
  TK_DECIMAL, TK_HEX
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
          case TK_HEX:
          case TK_DECIMAL: Assert(substr_len < 32, "token str too long!");
                           tokens[nr_token].type = rules[i].token_type;
                           memcpy(tokens[nr_token].str, substr_start, substr_len);            \
                           tokens[nr_token].str[substr_len] = '\0';                                       \
                           nr_token++; break;
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
  int rp = 0;
  int res = -1;

  for (int i = q; i >= p; i--) {
    if ((tokens[i].type == '+' || tokens[i].type == '-') && rp == 0) {
      return i;
    } else if (tokens[i].type == ')') {
      rp++;
    } else if (tokens[i].type == '(') {
      rp--;
    } else if ((tokens[i].type == '*' || tokens[i].type == '/') && rp == 0 && res == -1) {
      res = i;
    }
  }

  return res;
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
    else
      return strtoull(tokens[p].str, NULL, 16);
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
    int op = find_position(p, q);
    Assert(op > p, "no op between p and q");

    uint64_t val1 = eval(p, op - 1);
    uint64_t val2 = eval(op + 1, q);
    switch (tokens[op].type) {
      case '+': return val1 + val2;
      case '-': return val1 - val2;
      case '*': return val1 * val2;
      case '/': Assert(val2 != 0, "divide by zero!"); return val1 / val2;
      default: Assert(0, "invalid op");
    }
  }
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  *success = true;
  return eval(0, nr_token - 1);
}
