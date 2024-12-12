#include "syscall.h"

#include <common.h>

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

static Context* sc;

static uint32_t argraw(int n) {
  switch (n) {
    case 0:
      return sc->GPR2;
    case 1:
      return sc->GPR3;
    case 2:
      return sc->GPR4;
  }
  panic("argraw");
  return -1;
}

// Fetch the nth 32-bit system call argument.
static int argint(int n, int* ip) {
  *ip = argraw(n);
  return 0;
}

uint32_t sys_yield(void) {
  yield();
  return 0;
}

uint32_t sys_exit(void) {
  int n;
  argint(0, &n);
  halt(n);
  return 0;
}

static uint32_t (*syscalls[])(void) = {
    [SYS_exit] = sys_exit,
    [SYS_yield] = sys_yield,
};

void do_syscall(Context* c) {
  sc = c;
  int num = c->GPR1;

  if (num >= 0 && num < NELEM(syscalls) && syscalls[num]) {
    syscalls[num]();
  }
}
