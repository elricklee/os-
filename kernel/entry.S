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
/* lab1 - M 态准备：关中断、建栈、装 M 态陷阱向量，然后交给 start() */
#include "course_sid.h"

    .section .text
    .globl _entry
_entry:
    # 1) 关中断：启动早期现场未就绪，任何中断都可能破坏状态
    csrw mie, zero
    csrci mstatus, 8            # MSTATUS.MIE = 0

    # 2) 只让 0 号 hart 继续干活，其余 hart 原地 park（wfi 省电等待）
    csrr t0, mhartid
    bnez t0, park

    # 3) M 态陷阱向量：任何未预期异常都停在 park_trap
    #    mtvec 低 2 位是模式位，标号必须 4 字节对齐，否则写入被硬件静默丢弃
    la t0, park_trap
    csrw mtvec, t0

    # 4) 建立内核栈：栈底 stack0，容量由 LAB1_STACK_KB 决定（与 course_sid.h 强一致）
    la sp, stack0
    li t0, LAB1_STACK_KB * 1024
    add sp, sp, t0

    # 5) 进入 C：start() 里完成 M → S 切换
    call start

park:
    wfi
    j park

    .balign 4
park_trap:
    wfi
    j park_trap

    # 内核栈放在 .bss，不占镜像体积
    .section .bss
    .align 4
    .globl stack0
stack0:
    .space LAB1_STACK_KB * 1024