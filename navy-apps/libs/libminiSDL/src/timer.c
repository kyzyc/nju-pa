#include <NDL.h>
#include <SDL.h>
#include <sdl-timer.h>
#include <stdio.h>

uint32_t initTicks;

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
  panic("not implemented");
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  panic("not implemented");
  return 1;
}

uint32_t SDL_GetTicks() {
  return NDL_GetTicks() - initTicks;
}

void SDL_Delay(uint32_t ms) {
  panic("not implemented");
}
