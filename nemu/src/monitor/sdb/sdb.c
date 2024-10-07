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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include "sdb.h"
#include "debug.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

void print_wps();
WP* new_wp();
void free_wp(WP *wp, WP* parent);
WP* find_wp_with_index(int NO, WP** parent);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
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

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_si(char *args) {
  int step = 1;
  char *token = strtok(args, " ");

  if (token != NULL) {
    char *endptr;
    // 保证输入的是整数，而不是类似12ab或者1.34
    step = strtol(token, &endptr, 10);
    // 保证只能有一个额外参数
    token = strtok(NULL, " ");
    Assert((*endptr) == '\0', "step into arg failed!");
    Assert(token == NULL, "error near %s", args);
  }

  cpu_exec(step);

  return 0;
}

static int cmd_info(char *args) {
  int len = strlen(args);
  char *token = strtok(args, " ");

  Assert(token != NULL, "info command must has arg, either r or w");
  Assert(strlen(token) == len, "info only can have one argument!");

  if (strcmp(token, "r") == 0)
    isa_reg_display();
  else if (strcmp(token, "w") == 0) {
    print_wps();
  } else {
    panic("Unknown command argument '%s'\n", token);
  }

  return 0;
}

/**
(gdb) x/10 $pc
0x555555555131 <main+8>:        0x000000b8      0xf3c35d00      0x48fa1e0f      0x4808ec83
0x555555555141 <_fini+9>:       0xc308c483      0x00000000      0x00000000      0x00000000
0x555555555151: 0x00000000      0x00000000
 */
static int cmd_x(char *args) {
  char *token = strtok(args, " ");
  int N;

  Assert(token != NULL, "1: x command must have two arguments, usage: x <N> <EXPR>");

  char *endptr;
  N = strtol(token, &endptr, 10);
  Assert((*endptr) == '\0', "the first argument in x command must be a number!");

  bool success;
  word_t result = expr(endptr + 1, &success);
  if (success == false) {
    printf("A syntax error in expression, near `%s'.", endptr + 1);
  }

  for (int i = 0, j = 0; i < N; i++) {
    if (j == 0) {
      #ifdef CONFIG_ISA64
      printf("0x%-15lx", result + i * 4);
      #else
      printf("0x%-15x", result + i * 4);
      #endif
      
    }
    #ifdef CONFIG_ISA64
    printf("0x%08lx\t", vaddr_read(result + i * 4, 4));
    #else
    printf("0x%08x\t", vaddr_read(result + i * 4, 4));
    #endif
    
    if ((++j) == 4) {
      j = 0;
      printf("\n");
    }
  }
  if (N % 4 != 0) {
    printf("\n");
  }
  return 0;
}

static int cmd_p(char *args) {
  static int cnt = 0;

  bool success;
  word_t ret = expr(args, &success);

  if (success) {
    #ifdef CONFIG_ISA64
    printf("$%d = %lu\n", cnt++, ret);
    #else
    printf("$%d = %u\n", cnt++, ret);
    #endif
  } else {
    printf("expression evaluation failed!\n");
  }

  return 0;
}

static int cmd_w(char* args) {
  Assert(args != NULL, "watch command must have expression as argument");

  WP* nW = new_wp();

  nW->expr = (char*)malloc(sizeof(char) * (strlen(args) + 1));

  strcpy(nW->expr, args);

  bool success;
  nW->last_value = expr(nW->expr, &success);

  Assert(success == true, "give watchpoint initial value failed");

  #ifdef CONFIG_ISA64
  printf("watchpoint %d: %s, initial value: %lu\n", nW->NO, nW->expr, nW->last_value);
  #else
  printf("watchpoint %d: %s, initial value: %u\n", nW->NO, nW->expr, nW->last_value);
  #endif

  return 0;
}

static int cmd_d(char* args) {
  Assert(args != NULL, "delete command must have a number as argument");

  int NO;
  char *token = strtok(args, " ");

  if (token != NULL) {
    char *endptr;
    // 保证输入的是整数，而不是类似12ab或者1.34
    NO = strtol(token, &endptr, 10);
    // 保证只能有一个额外参数
    token = strtok(NULL, " ");
    Assert((*endptr) == '\0', "step into arg failed!");
    Assert(token == NULL, "error near %s", args);

    WP* parent;
    WP* cur = find_wp_with_index(NO, &parent);
    if (cur == NULL) {
      printf("No breakpoint number %d.\n", NO);
    } else {
      free_wp(cur, parent);
    }
  }

  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  { "si", "Step Into", cmd_si},
  { "info", "Print Program Status", cmd_info},
  { "x", "Scan Memory", cmd_x},
  { "p", "Expression Evaluation", cmd_p},
  { "w", "Setting Watchpoints", cmd_w},
  { "d", "Delete Watchpoint", cmd_d},
};

#define NR_CMD ARRLEN(cmd_table)

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

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
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

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
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

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
