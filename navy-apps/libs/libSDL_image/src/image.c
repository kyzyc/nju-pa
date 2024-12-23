#define SDL_malloc malloc
#define SDL_free free
#define SDL_realloc realloc

#define SDL_STBIMAGE_IMPLEMENTATION
#include <stdio.h>

#include "SDL_stbimage.h"

SDL_Surface *IMG_Load_RW(SDL_RWops *src, int freesrc) {
  assert(src->type == RW_TYPE_MEM);
  assert(freesrc == 0);
  return NULL;
}

SDL_Surface *IMG_Load(const char *filename) {
  FILE *fp = fopen(filename, "r");

  if (fp == NULL) {
    printf("fopen file %s failed\n", filename);
    assert(0);
  }

  fseek(fp, 0L, SEEK_END);
  uint32_t sz = ftell(fp);
  fseek(fp, 0L, SEEK_SET);

  char *buf = (char *)malloc((sz + 5) * sizeof(char));

  fread(buf, 1, sz, fp);

  SDL_Surface *ret = STBIMG_LoadFromMemory((const unsigned char *)buf, sz);

  assert(ret);

  fclose(fp);
  free(buf);

  return ret;
}

int IMG_isPNG(SDL_RWops *src) { return 0; }

SDL_Surface *IMG_LoadJPG_RW(SDL_RWops *src) { return IMG_Load_RW(src, 0); }

char *IMG_GetError() { return "Navy does not support IMG_GetError()"; }
