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

// #include "debug.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static int idx = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"#include <stdint.h>\n"
"int main() { "
"  uint64_t result = %s; "
"  printf(\"%%lu\", result); "
"  return 0; "
"}";

uint8_t choose(uint8_t n) {
  return rand() % n;
}

void gen_blank() {
  int numBlank = rand() % 5;

  for (int i = 0; i < numBlank; i++) {
    buf[idx++] = ' ';
  }
}

void gen_hex_num() {
  // 添加 (uint64_t) 前缀
  static const char hex_map[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8',\
         '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  const char *uint64_prefix = "(uint64_t)";
  int prefix_len = strlen(uint64_prefix);
  strcpy(&buf[idx], uint64_prefix);
  idx += prefix_len;
  buf[idx++] = '0';
  buf[idx++] = 'x';
  buf[idx++] = '0' + rand() % 2;
  int numBit = rand() % 9;

  for (int i = 0; i < numBit; i++) {
    buf[idx++] = hex_map[rand() % 16];
  }
  
}

void gen_decimal_num() {
  // 添加 (uint64_t) 前缀
  const char *uint64_prefix = "(uint64_t)";
  int prefix_len = strlen(uint64_prefix);
  strcpy(&buf[idx], uint64_prefix);
  idx += prefix_len;
  int numBit = (rand() % 18) + 1;
  buf[idx++] = '1' + rand() % 9;
  for (int i = 0; i < numBit; i++) {
    buf[idx++] = '0' + rand() % 10;
  }
}

void gen_num() {
  gen_blank();
  int ra = rand() % 2;

  if (ra == 0) {
    gen_decimal_num();
  } else {
    gen_hex_num();
  }
  gen_blank();
}

void gen(char c) {
  gen_blank();
  buf[idx++] = c;
  gen_blank();
}

static void gen_rand_expr();

void gen_rand_op() {
  gen_blank();
  switch (choose(7)) {
    case 0:  buf[idx++] = '+'; break;
    case 1:  buf[idx++] = '-'; break;
    case 2:  buf[idx++] = '*'; break;
    case 3:  buf[idx++] = '='; buf[idx++] = '='; break;
    case 4:  buf[idx++] = '!'; buf[idx++] = '='; break;
    case 5:  buf[idx++] = '&'; buf[idx++] = '&'; break;
    default: buf[idx++] = '/'; break;
  }
  gen_blank();
}

static void gen_rand_expr() {
  if (65536 - idx < 10000) {
    gen_num();
    return;
  }
  switch (choose(3)) {
    case 0:   gen_num(); break;
    case 1:   gen('('); gen_rand_expr(); gen(')'); break;
    default:  gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    idx = 0;
    gen_rand_expr();
    buf[idx] = '\0';
    assert(idx < 65536);

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc -Werror=div-by-zero /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    uint64_t result;
    ret = fscanf(fp, "%lu", &result);
    ret = pclose(fp);

    if (ret >= 0)
    printf("%lu %s\n", result, buf);
  }
  return 0;
}
