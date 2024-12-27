#include <NDL.h>
#include <SDL.h>

extern uint32_t initTicks;

int SDL_Init(uint32_t flags) {
  // panic("not implemented");
  initTicks = NDL_GetTicks();
  return NDL_Init(flags);
}

void SDL_Quit() {
  // panic("not implemented");
  NDL_Quit();
}

char *SDL_GetError() {
  panic("not implemented");
  return "Navy does not support SDL_GetError()";
}

int SDL_SetError(const char* fmt, ...) {
  panic("not implemented");
  return -1;
}

int SDL_ShowCursor(int toggle) {
  panic("not implemented");
  return 0;
}

void SDL_WM_SetCaption(const char *title, const char *icon) {
  // printf("titie:%s\n", title);
  // panic("not implemented");
}
