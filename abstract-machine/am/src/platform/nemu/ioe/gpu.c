#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

static struct canvas {
  uint16_t w, h;
  uint32_t size;
} display;

struct pixel {
  uint8_t b, g, r;
} __attribute__ ((packed));

void __am_gpu_init() {
  uint32_t wh = inl(VGACTL_ADDR);
  display.w = wh >> 16;
  display.h = wh & 0x0000ffff;
  display.size = display.w * display.h * sizeof(uint32_t);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = display.w, .height = display.h,
    .vmemsz = display.size
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  int W = display.w, H = display.h;
  int len = (x + w >= W) ? W - x : w;
  uint32_t* pixels = ctl->pixels;
  for (int j = 0; j < h; j++, pixels += w) {
    if (y + j < H) {
      uint32_t offset = FB_ADDR + ((j + y) * W + x) * sizeof(uint32_t);
      for (int i = 0; i < len; i++) {
        uint32_t p = pixels[i];
        // sprintf(stderr, "write to %x\n", FB_ADDR + ((j + y) * W + x + i));
        outl(offset + i * sizeof(uint32_t), p);
      }
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_memcpy(AM_GPU_MEMCPY_T* params) {
  char *src = params->src;
  uint32_t dst = params->dest + (uint32_t)FB_ADDR;

  for (int i = 0; i < params->size && i < display.size; i++) {
    outb(dst + i, src[i]);
  }
}



void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
