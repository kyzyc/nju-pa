#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <sdl-event.h>
#include <string.h>

#define keyname(k) #k,

#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

static const char *keyname[] = {"NONE", _KEYS(keyname)};

#define SDL_RELEASED 0
#define SDL_PRESSED 1
static uint8_t keyState[NELEM(keyname)];

int SDL_PushEvent(SDL_Event *ev) {
  panic("not implemented");
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  char buf[64];
  if (NDL_PollEvent(buf, sizeof(buf))) {
    if (buf[0] == 'k') {
      ev->type = (buf[1] == 'd' ? SDL_KEYDOWN : SDL_KEYUP);
      for (int i = 0; i < NELEM(keyname); i++) {
        if (strcmp(buf + 3, keyname[i]) == 0) {
          keyState[i] = (ev->type == SDL_KEYDOWN ? 1 : 0);
          ev->key.keysym.sym = i;
          return 1;
        }
      }
      assert(0);
    } else {
      return 0;
    }
  }
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  while (1) {
    char buf[64];
    if (NDL_PollEvent(buf, sizeof(buf))) {
      if (buf[0] == 'k') {
        event->type = (buf[1] == 'd' ? SDL_KEYDOWN : SDL_KEYUP);
        for (int i = 0; i < NELEM(keyname); i++) {
          if (strcmp(buf + 3, keyname[i]) == 0) {
            keyState[i] = (event->type == SDL_KEYDOWN ? 1 : 0);
            event->key.keysym.sym = i;
            return 1;
          }
        }
        assert(0);
      } else {
      }
      break;
    }
  }

  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  panic("not implemented");
  return 0;
}

uint8_t *SDL_GetKeyState(int *numkeys) {
  // panic("not implemented");
  return keyState;
}
