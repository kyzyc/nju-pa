#include <NDL.h>
#include <SDL.h>

int SDL_OpenAudio(SDL_AudioSpec *desired, SDL_AudioSpec *obtained) {
  // panic("not implemented");
  return 0;
}

void SDL_CloseAudio() {
}

void SDL_PauseAudio(int pause_on) {
}

void SDL_MixAudio(uint8_t *dst, uint8_t *src, uint32_t len, int volume) {
  panic("not implemented");
}

SDL_AudioSpec *SDL_LoadWAV(const char *file, SDL_AudioSpec *spec, uint8_t **audio_buf, uint32_t *audio_len) {
  return NULL;
}

void SDL_FreeWAV(uint8_t *audio_buf) {
  panic("not implemented");
}

void SDL_LockAudio() {
  // panic("not implemented");
}

void SDL_UnlockAudio() {
  // panic("not implemented");
}
