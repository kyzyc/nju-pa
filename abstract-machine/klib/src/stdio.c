#include <am.h>
#include <klib-macros.h>
#include <klib.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  char buf[512] = ""; // why must initialize???
  va_list args;
  va_start(args, fmt);

  int ret = vsprintf(buf, fmt, args);
  va_end(args);

  putstr(buf);

  return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  union {
    int iarg;
    const char *sarg;
    char carg;
  } arg;
  char *q = out;
  int n;

  for (const char *p = fmt; (*p) != '\0'; p++) {
    if ((*p) == '%') {
      p++;
      switch (*p) {
        case 'd':
          arg.iarg = va_arg(ap, int);
          if (arg.iarg < 0) {
            *q = '-';
            q++;
            arg.iarg = (-arg.iarg);
          } else if (arg.iarg == 0) {
            *q = '0';
            q++;
          }
          n = itoa(arg.iarg, q, 10, 0);
          q += n;
          break;
        case 's':
          arg.sarg = va_arg(ap, const char *);
          n = strlen(arg.sarg);
          strcat(q, arg.sarg);
          q += (n);
          break;
        case 'x':
          arg.iarg = va_arg(ap, int);
          if (arg.iarg == 0) {
            *q = '0';
            q++;
          }
          n = itoa(arg.iarg, q, 16, 1);
          q += n;
          break;
        case 'c':
          arg.carg = va_arg(ap, int);
          *q = arg.carg;
          q++;
          break;
        default:
          *q = *p;
          q++;
          // goto out;
          // panic("unknown format specifer");
          break;
      }
    } else {
      *q = *p;
      q++;
    }
  }
// out:
  *q = '\0';

  return q - out;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  return vsprintf(out, fmt, ap);
}


int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
