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
#include "local-include/reg.h"

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};


/**
rax            0x555555555129      93824992235817                                                                       rbx            0x7fffffffe4b8      140737488348344                                                                      rcx            0x555555557df8      93824992247288                                                                       rdx            0x7fffffffe4c8      140737488348360
rsi            0x7fffffffe4b8      140737488348344
 */
#define NR_REGS_TABLE ARRLEN(regs)
void isa_reg_display() {
  #ifdef CONFIG_ISA64
    printf("%-15s %-15lx %-15ld\n", "pc", cpu.pc, cpu.pc);
  #else
    printf("%-15s %-15x %-15d\n", "pc", cpu.pc, cpu.pc);
  #endif
  for (int i = 0; i < NR_REGS_TABLE; i++) {
    #ifdef CONFIG_ISA64
    printf("%-15s %-15lx %-15ld\n", regs[i], cpu.gpr[i], cpu.gpr[i]);
  #else
    printf("%-15s %-15x %-15d\n", regs[i], cpu.gpr[i], cpu.gpr[i]);
  #endif
  }
}

word_t isa_reg_str2val(const char *s, bool *success) {
  if (strcmp(s, "pc") == 0) {
    *success = true;
    return cpu.pc;
  }

  for (int i = 0; i < NR_REGS_TABLE; i++) {
    if (strcmp(s, regs[i]) == 0) {
      *success = true;
      return cpu.gpr[i];
    }
  }
  
  *success = false;
  return 0;
}
