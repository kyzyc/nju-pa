#include <NDL.h>
#include <stdio.h>
#include <sys/time.h>

int main() {
  struct timeval tv;
  int i = 0;
  NDL_Init(0);
  uint32_t before = NDL_GetTicks();
  uint32_t after = before;

  while (1) {
    while (after - before < 500) {
      after = NDL_GetTicks();
    }

    before = after;

    printf("%dth half-seconds\n", ++i);
  }

  return 0;
}
