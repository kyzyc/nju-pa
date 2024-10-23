#include <klib-macros.h>
#include <klib.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)


int itoa(int num, char *dst, int base, int type) {
  static char map[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  char number[32];
  int idx = 0;

  if (type == 1) {
    for (uint32_t unum = num; unum; unum /= base)
      number[idx++] = map[(unum % base)];
  } else {
    for (; num; num /= base) 
      number[idx++] = map[(num % base)];
  }

  int cnt = idx;

  for (idx = idx - 1; idx >= 0; idx--, dst++) {
    *dst = number[idx];
  }

  return cnt;
}

size_t strlen(const char *s) {
  size_t res = 0;

  while ((*s) != '\0') {
    s++;
    res++;
  }

  return res;
}

char *strcpy(char *dst, const char *src) {
  char *p = dst;
  for (; *src != '\0'; p++, src++) {
    *p = *src;
  }
  *p = '\0';
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  char *p = dst;
  while ((*p) != '\0') p++;
  for (; *src != '\0'; p++, src++) {
    *p = *src;
  }
  *p = '\0';
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  const char *p = s1, *q = s2;
  while ((*p) != '\0' && (*p) == (*q)) {
    p++;
    q++;
  }

  if ((*p) == (*q)) {
    return 0;
  } else if ((*p) > (*q)) {
    return 1;
  }

  return -1;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  char *p = s;
  for (size_t i = 0; i < n; i++, p++) {
    (*p) = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  return memcpy(dst, src, n);
}

void *memcpy(void *out, const void *in, size_t n) { 
  char *p = out;
  const char *q = in;
  for (size_t i = 0; i < n; i++, p++, q++) {
    (*p) = (*q);
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const char *p = s1, *q = s2;
  size_t i;
  for (i = 0; (*p) == (*q) && i < n; i++, p++, q++);

  if (i == n || (*p) == (*q)) {
    return 0;
  } else if ((*p) > (*q)) {
    return 1;
  }
  return -1;
}

#endif
