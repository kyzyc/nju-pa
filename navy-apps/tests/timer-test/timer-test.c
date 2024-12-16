#include <stdio.h>
#include <sys/time.h>

int main() {
  struct timeval tv;
  int i = 0;
  gettimeofday(&tv, NULL);
  long before = tv.tv_sec * 1000 + tv.tv_usec / 1000;;
  long after = before;

  while (1) {
    while (after - before < 500) {
      gettimeofday(&tv, NULL);
      after = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }

    before = after;

    printf("%dth half-seconds\n", ++i);
  }

  return 0;
}
