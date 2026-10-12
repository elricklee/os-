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
/* lab1 - 轮询式 UART 输出 + 空转节流 + 个人行协议 */
#include "types.h"
#include "memlayout.h"
#include "course_sid.h"

#define THR 0                    /* 发送保持寄存器（写） */
#define LSR 5                    /* 线路状态寄存器 */
#define LSR_THRE (1 << 5)        /* bit5：THR 空 */

#define UART_BASE ((volatile unsigned char *)UART0)

/* 节流周期 = 16 + 学号 % 16 字节（引用 COURSE_SID，严禁硬编码数字） */
#define THROTTLE_PERIOD (16 + COURSE_SID % 16)

/* 控制台最底层：只负责单字节硬件发射。
 * 忙等 LSR.THRE 置位后写 THR；启动期无时钟中断，忙等是唯一可靠行为。
 * 仅单核单上下文调用，声明为不可重入。 */
void
uartputc_sync(char c)
{
  while ((UART_BASE[LSR] & LSR_THRE) == 0)
    ;
  UART_BASE[THR] = c;
}

/* 控制台层：每输出 THROTTLE_PERIOD 字节插入一次 nop 空转 */
void
putchar(char c)
{
  static int n;                  /* 已输出字节数 */
  uartputc_sync(c);
  if (++n >= THROTTLE_PERIOD) {
    n = 0;
    for (volatile int i = 0; i < THROTTLE_PERIOD; i++)
      asm volatile("nop");
  }
}

/* 个人行协议：LAB1_BANNER_PROTOCOL == 1 —— 每个字节后紧跟一个 '.' */
void
line_putc(char c)
{
#if LAB1_BANNER_PROTOCOL == 1
  putchar(c);
  putchar('.');
#else
  putchar(c);
#endif
}