#include <fixedptc.h>
#include <stdio.h>
#include <assert.h>

int main() {
  assert(fixedpt_floor(fixedpt_rconst(1.2)) == fixedpt_rconst(1));
  assert(fixedpt_ceil(fixedpt_rconst(1.2)) == fixedpt_rconst(2));
  assert(fixedpt_floor(fixedpt_rconst(-1.2)) == fixedpt_rconst(-2));
  assert(fixedpt_ceil(fixedpt_rconst(-1.2)) == fixedpt_rconst(-1));

  return 0;
}
