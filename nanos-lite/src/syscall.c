#include "syscall.h"

#include <common.h>
#include <sys/time.h>

#include "fs.h"

// #define CONFIG_STRACE

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

uint32_t sys_open(void) {
  uint32_t addr;
  int flags, mode;
  if (argaddr(0, &addr) < 0 || argint(1, &flags) < 0 || argint(2, &mode) < 0) {
    return -1;
  }

  return fs_open((const char*)addr, flags, mode);
}

uint32_t sys_exit(void) {
  int n;
  if (argint(0, &n) < 0) {
    return -1;
  }
  halt(n);
  return 0;
}

uint32_t sys_read(void) {
  int fd, n;
  uint32_t addr;
  if (argint(0, &fd) < 0 || argaddr(1, &addr) < 0 || argint(2, &n) < 0) {
    return -1;
  }

  return fs_read(fd, (char*)addr, n);
}

uint32_t sys_write(void) {
  int fd, n;
  uint32_t addr;
  if (argint(0, &fd) < 0 || argaddr(1, &addr) < 0 || argint(2, &n) < 0) {
    return -1;
  }

  return fs_write(fd, (const char*)addr, n);
}

uint32_t sys_close(void) {
  int fd;

  if (argint(0, &fd) < 0) {
    return -1;
  }

  return fs_close(fd);
}

uint32_t sys_lseek(void) {
  int fd, whence, off_t;

  if (argint(0, &fd) < 0 || argint(1, &off_t) < 0 || argint(2, &whence) < 0) {
    return -1;
  }

  return fs_lseek(fd, off_t, whence);
}

uint32_t sys_brk(void) {
  uint32_t addr;

  if (argaddr(0, &addr) < 0) {
    return -1;
  }

  // do something

  return 0;
}

uint32_t sys_gettimeofday(void) {
  uint32_t addr;

  if (argaddr(0, &addr) < 0) {
    return -1;
  }

  struct timeval* tv = (struct timeval*)addr;

  uint32_t microsecs = io_read(AM_TIMER_UPTIME).us; 

  tv->tv_sec = microsecs / 1000000;
  tv->tv_usec = microsecs % (1000000);

  return 0;
}

static uint32_t (*syscalls[])(void) = {[SYS_exit] = sys_exit,
                                       [SYS_yield] = sys_yield,
                                       [SYS_open] = sys_open,
                                       [SYS_read] = sys_read,
                                       [SYS_write] = sys_write,
                                       [SYS_close] = sys_close,
                                       [SYS_lseek] = sys_lseek,
                                       [SYS_brk] = sys_brk,
                                       [SYS_gettimeofday] = sys_gettimeofday};

#ifdef CONFIG_STRACE
const static char* sys_call_name_index[] = {
    [SYS_exit] = "exit",
    [SYS_yield] = "yield",
    [SYS_open] = "open",
    [SYS_read] = "read",
    [SYS_write] = "write",
    [SYS_close] = "close",
    [SYS_lseek] = "lseek",
    [SYS_brk] = "brk",
    [SYS_gettimeofday] = "gettimeofday"};
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
