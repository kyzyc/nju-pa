#include <NDL.h>
#include <sdl-timer.h>
#include <stdio.h>

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
  panic("not implemented");
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  panic("not implemented");
  return 1;
}

uint32_t SDL_GetTicks() {
  panic("not implemented");
  return 0;
}

void SDL_Delay(uint32_t ms) {
  panic("not implemented");
}
