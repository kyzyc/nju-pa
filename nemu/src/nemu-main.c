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

#include <common.h>

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();

// #define TEST_EXPR
#undef TEST_EXPR

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

#ifdef TEST_EXPR
#include "monitor/sdb/sdb.h"
#include <stdio.h>
  FILE *input_file = fopen("/home/zyc/ics2024/nemu/tools/gen-expr/build/input", "r");
  if (input_file == NULL) {
      perror("无法打开 input 文件");
      return -1;
  }
  static char* buf, *buf2;
  size_t buf_size = 65536;
  ssize_t read;
  buf = malloc(buf_size * sizeof(char));
  buf2 = malloc(buf_size * sizeof(char));
  uint64_t expected_result;

  bool success; 
  int cnt = 0;
  while ((read = getline(&buf, &buf_size, input_file)) >= 0) {
      if (buf[read - 1] != '\n') {
        panic("yes\n");
      }
      buf[read - 1] = '\0';
      sscanf(buf, "%lu %[^\n]", &expected_result, buf2);
      // 调用 expr() 函数求值
      uint64_t result = expr(buf2, &success);
      cnt ++;
      // 比较结果
      Assert(success == true, "expr not success");
      Assert(result == expected_result, "result not equal, expr:%s \nresult:%lu\t expect: %lu\n", buf, result, expected_result);
  }
  printf("all tests success! %d", cnt);
  free(buf);
  free(buf2);
#else
  /* Start engine. */
  engine_start();
#endif

  return is_exit_status_bad();
}
