#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) { panic("Not implemented"); }

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  union {
    int iarg;
    const char *carg;
  } arg;
  char *q = out;
  int n;

  va_start(ap, fmt);

  for (const char *p = fmt; (*p) != '\0'; p++) {
    if ((*p) == '%') {
      p++;
      switch (*p) {
        case 'd':
          arg.iarg = va_arg(ap, int);
          n = itoa(arg.iarg, q);
          q += n;
          break;
        case 's':
          arg.carg = va_arg(ap, const char *);
          n = strlen(arg.carg);
          strcat(q, arg.carg);
          q += (n);
          break;
        default:
          panic("unknown format specifer");
          break;
      }
    } else {
      *q = *p;
      q++;
    }
  }

  return q - out;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
