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

#include "sdb.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].expr = NULL;
    wp_pool[i].last_value = 0;
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */
WP* new_wp() {
  if (free_ == NULL) {
    // 没有空闲链表
    assert(0);
  }

  WP* p = free_;
  free_ = free_->next;

  p->next = head;
  head = p;

  return p;
}

void free_wp(WP *wp) {
  free(wp->expr);
  wp->expr = NULL;
  wp->last_value = 0;

  wp->next = free_;
  free_ = wp;
}

uint8_t detect_wp_change(CWP** vec_wp) {
  Assert(*vec_wp == NULL, "vector of watchpoints should be empty");

  uint8_t size = 0;

  for (WP* cur = head; cur != NULL; cur = cur->next) {
    bool success;
    uint64_t ret = expr(cur->expr, &success);

    // printf("ret: %lu\n", ret);
    // printf("expr: %s\n", cur->expr);
    // printf("last_value: %lu\n", cur->last_value);
    // printf("NO: %d\n", cur->NO);

    Assert(success == true, "expression evaluation failed");

    if (ret != cur->last_value) {
      *vec_wp = (CWP*)realloc(*(vec_wp), (++size) * sizeof(CWP));
      (*vec_wp)[size - 1].wp = cur;
      (*vec_wp)[size - 1].old_value = cur->last_value;
      cur->last_value = ret;
      if (nemu_state.state == NEMU_RUNNING)
        nemu_state.state = NEMU_STOP;
    }
  }

  return size;
}


