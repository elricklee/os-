/*
 * lab1 初始骨架代码(自动生成): 系统启动与串口控制台输出。
 * 启动至此的前期初始化流程，需要由你在本实验中设计并实现。
 * 你需要实现: entry.S(start 前的 M 态准备可另置 start.c)、串口轮询输出、
 * 最小 printf。链接脚本 kernel.ld 带注释保留; 底层宏 riscv.h 完整保留。
 * 代码导读路线与设计引导问题详见《实验说明书(lab1)》。
 *
 * 两个环境注意事项(说明书 §2"环境前置条件"与附录 C, 动手前必读):
 *  1. start() 的 M→S 切换清单必须包含 PMP 配置(最简两行):
 *       w_pmpaddr0(0x3fffffffffffffull); w_pmpcfg0(0xf);
 *     否则在新版 QEMU 上 mret 进 S 态的第一条取指即触发 fault(全程无输出)。
 *  2. entry.S 里的陷阱向量标号前加 .balign 4(mtvec 要求 4 字节对齐,
 *     不满足时写入会被硬件静默丢弃)。
 */
/* lab1 - 最小 printf：%d %x %s %c %p %% 与 %l 长度修饰 */
#include <stdarg.h>
#include "types.h"

void line_putc(char c);

static const char digits[] = "0123456789abcdef";

static void
printnum(unsigned long long x, int base)
{
  char buf[24];
  int i = 0;

  if (x == 0)
    buf[i++] = '0';
  while (x > 0) {
    buf[i++] = digits[x % base];
    x /= base;
  }
  while (i-- > 0)
    line_putc(buf[i]);
}

static void
printint(long long xx, int base, int sign)
{
  if (sign && xx < 0) {
    line_putc('-');
    printnum((unsigned long long)-xx, base);
  } else {
    printnum((unsigned long long)xx, base);
  }
}

static void
printptr(unsigned long long x)
{
  line_putc('0');
  line_putc('x');
  for (int i = 15; i >= 0; i--)
    line_putc(digits[(x >> (i * 4)) & 0xf]);
}

void
printf(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  for (const char *p = fmt; *p != 0; p++) {
    if (*p != '%') {
      line_putc(*p);
      continue;
    }
    p++;
    int l = 0;
    if (*p == 'l') {             /* %ld / %lx */
      l = 1;
      p++;
    }
    switch (*p) {
    case 'd':
      if (l) printint(va_arg(ap, long), 10, 1);
      else   printint(va_arg(ap, int), 10, 1);
      break;
    case 'x':
      if (l) printint(va_arg(ap, unsigned long), 16, 0);
      else   printint(va_arg(ap, unsigned int), 16, 0);
      break;
    case 'p':
      printptr(va_arg(ap, unsigned long long));
      break;
    case 's': {
      char *s = va_arg(ap, char *);
      if (s == 0)
        s = "(null)";
      while (*s)
        line_putc(*s++);
      break;
    }
    case 'c':
      line_putc((char)va_arg(ap, int));
      break;
    case '%':
      line_putc('%');
      break;
    default:                     /* 未知格式原样输出，不崩溃 */
      line_putc('%');
      line_putc(*p);
      break;
    }
  }
  va_end(ap);
}