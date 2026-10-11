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
/* lab1 - 接过 M 态，完成 M → S 切换后交给 main() */
#include "types.h"
#include "riscv.h"
#include "course_sid.h"

void main(void);

void
start(void)
{
  /* 此时仍在 M 态，sp 已由 entry.S 建好 */

  /* 1) PMP：授权 S 态访问全部物理地址。
   *    新版 QEMU 上缺这两行，mret 进 S 态后第一条取指就 fault，现象是全程黑屏。
   *    说明书 §2 明确允许直接照抄这两行。 */
  w_pmpaddr0(0x3fffffffffffffull);
  w_pmpcfg0(0xf);

  /* 2) 中断与委托：lab1 不处理任何陷阱，先全部关掉
   *    （陷阱分发、stvec 是 lab2 的内容） */
  w_mie(0);
  w_medeleg(0);
  w_mideleg(0);

  /* 3) mret 的返回目标：main() */
  w_mepc((uint64)main);

  /* 4) mstatus：MPP = S（返回后进入 S 态），并保持 M 态中断关闭 */
  uint64 x = r_mstatus();
  x &= ~MSTATUS_MPP_MASK;
  x |= MSTATUS_MPP_S;
  x &= ~(1L << 3);            /* MSTATUS.MIE = 0（riscv.h 未提供该宏） */
  w_mstatus(x);

  /* 5) satp 保持 0（Bare 模式）：lab1 还没有页表，直接物理寻址 */
  asm volatile("mret");
}