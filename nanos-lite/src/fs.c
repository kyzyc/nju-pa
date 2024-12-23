#include <fs.h>

typedef size_t (*ReadFn)(void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn)(const void *buf, size_t offset, size_t len);

extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);

extern size_t serial_write(const void *buf, size_t offset, size_t len);

extern size_t events_read(void *buf, size_t offset, size_t len);

extern size_t dispinfo_read(void *buf, size_t offset, size_t len);

extern size_t fb_write(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum { FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_KB, FD_DISPINFO };

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
    [FD_STDIN] = {"stdin", 0, 0, invalid_read, invalid_write},
    [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
    [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
    [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write},
    [FD_KB] = {"/dev/events", 0, 0, events_read, invalid_write},
    [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int w = io_read(AM_GPU_CONFIG).width;
  int h = io_read(AM_GPU_CONFIG).height;

  file_table[FD_FB].size = w * h * sizeof(uint32_t);
}

int fs_open(const char *pathname, int flags, int mode) {
  int sz = NELEM(file_table);
  for (int i = 0; i < sz; i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      return i;
    }
  }

  printf("open %s filed\n", pathname);
  assert(0);  // should not reach here.
  return -1;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  switch (whence) {
    case SEEK_CUR:
      file_table[fd].open_offset += offset;
      break;
    case SEEK_SET:
      file_table[fd].open_offset = offset;
      break;
    case SEEK_END:
      assert(file_table[fd].size + offset <= file_table[fd].size);
      file_table[fd].open_offset = file_table[fd].size + offset;
      break;
    default:
      assert(0);
      break;
  }
  return file_table[fd].open_offset;
}

size_t fs_read(int fd, void *buf, size_t len) {
  if (len <= 0) {
    return 0;
  } else if (file_table[fd].read) {
    return file_table[fd].read(buf, file_table[fd].open_offset, len);
  }

  assert(fd < NELEM(file_table));

  int cnt = (file_table[fd].open_offset + len < file_table[fd].size)
                ? len
                : (file_table[fd].size - 1 - file_table[fd].open_offset);

  ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset,
               cnt);

  file_table[fd].open_offset += cnt;

  return cnt;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  if (len <= 0) {
    return 0;
  } else if (file_table[fd].write) {
    return file_table[fd].write(buf,file_table[fd].open_offset, len);
  }

  assert(fd < NELEM(file_table));

  int cnt = (file_table[fd].open_offset + len < file_table[fd].size)
                ? len
                : (file_table[fd].size - 1 - file_table[fd].open_offset);

  ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset,
                cnt);

  file_table[fd].open_offset += cnt;
  return cnt;
}

int fs_close(int fd) { return 0; }