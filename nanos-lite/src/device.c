#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
#define MULTIPROGRAM_YIELD() yield()
#else
#define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) [AM_KEY_##key] = #key,

static const char *keyname[256]
    __attribute__((used)) = {[AM_KEY_NONE] = "NONE", AM_KEYS(NAME)};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  int i = 0;
  for (; i < len; i++) {
    putch(*((char *)buf + i));
  }
  return i;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  if (ev.keycode == AM_KEY_NONE) {
    return 0;
  }

  // TODO: use snprintf
  return sprintf((char *)buf, "k%c %s\n", ev.keydown ? 'd' : 'u',
                 keyname[ev.keycode]);
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  int w = io_read(AM_GPU_CONFIG).width;
  int h = io_read(AM_GPU_CONFIG).height;

  return sprintf(buf, "%s: %d\n%s: %d\n", "WIDTH", w, "HEIGHT", h);
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int w = io_read(AM_GPU_CONFIG).width;

  int x = (offset / 4) % w;
  int y = (offset / 4) / w;

  io_write(AM_GPU_FBDRAW, x, y, (uint32_t *)buf, len / 4, 1, true);

  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
