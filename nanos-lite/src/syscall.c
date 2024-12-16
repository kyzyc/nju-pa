#include "syscall.h"

#include <common.h>

// #define CONFIG_STRACE

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

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
int argaddr(int n, uint32_t* ip) {
  *ip = argraw(n);
  return 0;
}

uint32_t sys_yield(void) {
  yield();
  return 0;
}

uint32_t sys_exit(void) {
  int n;
  if (argint(0, &n) < 0) {
    return -1;
  }
  halt(n);
  return 0;
}

uint32_t sys_write(void) {
  int fd, n;
  uint32_t addr;
  if (argint(0, &fd) < 0 || argaddr(1, &addr) < 0 || argint(2, &n) < 0) {
    return -1;
  }

  if (fd == 1 || fd == 2) {
    int i;
    for (i = 0; i < n; i++) {
      putch(*(char*)(addr + i));
    }
    return i;
  } else {
    return -1;
  }
}

uint32_t sys_brk(void) {
  uint32_t addr;

  if (argaddr(0, &addr) < 0) {
    return -1;
  }

  // do something

  return 0;
}

static uint32_t (*syscalls[])(void) = {
    [SYS_exit] = sys_exit,
    [SYS_yield] = sys_yield,
    [SYS_write] = sys_write,
    [SYS_brk] = sys_brk,
};

#ifdef CONFIG_STRACE
const static char* sys_call_name_index[] = {
    [SYS_exit] = "exit",
    [SYS_yield] = "yield",
    [SYS_write] = "write",
    [SYS_brk] = "brk",
};
#endif

void do_syscall(Context* c) {
  sc = c;
  int num = c->GPR1;

  if (num >= 0 && num < NELEM(syscalls) && syscalls[num]) {
    c->GPRx = syscalls[num]();
#ifdef CONFIG_STRACE
    printf("syscall %s -> %d\n", sys_call_name_index[num], c->GPRx);
#endif
  }
}
