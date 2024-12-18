#include <assert.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int frame_w = 0, frame_h = 0;
static int bias_w = 0, bias_h = 0;
static int kb_fd;
static int dispinfo_fd;

uint32_t NDL_GetTicks() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

int NDL_PollEvent(char *buf, int len) { return read(kb_fd, buf, len); }

static void NDL_ParseDispInfo() {
  char buf[64];
  int nread = read(dispinfo_fd, buf, sizeof(buf) - 1);

  char *p = buf;
  char *target = NULL;

  while (*p) {
    while (*p == ' ' || *p == '\t' || *p == '\n') {
      p++;
    }

    if (*p == 'W' && *(p + 1) == 'I' && *(p + 2) == 'D' && *(p + 3) == 'T' &&
        *(p + 4) == 'H') {
      target = p;
      p += 5;
    } else if (*p == 'H' && *(p + 1) == 'E' && *(p + 2) == 'I' &&
               *(p + 3) == 'G' && *(p + 4) == 'H' && *(p + 5) == 'T') {
      target = p;
      p += 6;
    } else {
      while (*p != '\n' && *p != '\0') {
        p++;
      }
      continue;
    }

    while (*p == ' ' || *p == '\t' || *p == ':') {
      p++;
    }

    int value = 0;
    while (*p >= '0' && *p <= '9') {
      value = value * 10 + (*p - '0');
      p++;
    }

    // 根据 key 更新值
    if (*target == 'H') {
      frame_h = value;
    } else {
      frame_w = value;
    }
  }

  printf("frame_w: %d\nframe_h: %d\n", frame_w, frame_h);
}

void NDL_OpenCanvas(int *w, int *h) {
  dispinfo_fd = open("/proc/dispinfo", 0, 0);
  NDL_ParseDispInfo();
  screen_w = *w;
  screen_h = *h;

  bias_h = (frame_h - screen_h) / 2;
  bias_w = (frame_w - screen_w) / 2;

  printf("screen_w: %d\nscreen_h: %d\n", screen_w, screen_h);

  if (getenv("NWM_APP")) {
    assert(*w <= screen_w);
    assert(*h <= screen_h);
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w;
    screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  for (int j = 0; j < h; j++) {
    lseek(3, ((y + j + bias_h) * frame_w) + x + bias_w, SEEK_SET);
    write(3, pixels + j * w, w);
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {}

void NDL_CloseAudio() {}

int NDL_PlayAudio(void *buf, int len) { return 0; }

int NDL_QueryAudio() { return 0; }

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }

  kb_fd = open("/dev/events", 0, 0);

  return 0;
}

void NDL_Quit() {}
