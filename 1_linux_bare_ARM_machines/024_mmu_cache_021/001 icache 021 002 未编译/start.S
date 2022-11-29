
.text
.global _start

_start:
	/***
	address exception 
	0x00    Reset
	0x04    Undefined instruction
	0x08    Software Interrupt
	0x0c    Abort (prefetch)
	0x10    Abort (data)
	0x14    Reserved
	0x18    IRQ
	0x1c    FIQ
	***/
	b reset /* vector 0: reset */
	//b do_und/* vector 4: und */
	ldr pc, und_addr/* vector 4: und */
	ldr pc, swi_addr/* vector 4: und */
	b halt
	b halt
	b halt
	ldr pc, irq_addr/* vector 4: und */
	b halt

und_addr:
	.word do_und/* 使得异常处理语句地址在4k以内，不管是Nor还是Nand都可以 */

swi_addr:
	.word do_swi/* 使得异常处理语句地址在4k以内，不管是Nor还是Nand都可以 */

irq_addr:
	.word do_irq/* 使得异常处理语句地址在4k以内，不管是Nor还是Nand都可以 */

do_und:/* 异常处理 */

	/* 执行到这里之前:
	 * 1. lr_und保存被中断模式中的下一条即将执行的指令的地址
	 * 2. SPSR_und保存被中断模式的CPSR
	 * 3. CPSR中的M4-M0被设置为11011, 进入到und模式
	 * 4. 跳到0x4的地方执行程序 
	 */

	
	/* 异常模式下的sp没有设置，得设置 */
	ldr sp, =0x34000000

	/*在und异常处理函数中有可能会修改r0- r12，所以先保存起来 */
	stmdb sp!, {r0-r12, lr}/* 还有lr，即异常处理完后的返回地址 */
	
	/* 保存现场 */
	/* 处理und异常 */
	mrs r0, cpsr/* 传printException的第一个实参 */
	ldr r1, =und_string/* 传printException的第二个实参 */
	bl printException/* 如果是Nand启动，printException函数在4K之外的话，
						这里就会出错，优化办法：将相对跳转到do_und改为
						绝对跳转*/

	/* 恢复现场 */
	ldmia sp!, {r0-r12, pc}^ /* ^符号会把spsr的值恢复到cpsr里 */

und_string:
	.string "undefined instruction exception"

.align 4/* 为避免受前面字符串长度的影响，加这条语句可以保证后面一条指令的地址是4字节对齐 */


do_swi:/* 异常处理 */

	/* 执行到这里之前:
	 * 1. lr_svr保存被中断模式中的下一条即将执行的指令的地址
	 * 2. SPSR_svr保存被中断模式的CPSR
	 * 3. CPSR中的M4-M0被设置为10011, 进入到svr模式
	 * 4. 跳到0x8的地方执行程序 
	 */

	
	/* 异常模式下的sp没有设置，得设置 */
	ldr sp, =0x33e00000

	/*在und异常处理函数中有可能会修改r0- r12，所以先保存起来 */
	stmdb sp!, {r0-r12, lr}/* 还有lr，即异常处理完后的返回地址 */

	mov r4, lr
	
	/* 保存现场 */
	/* 处理und异常 */
	mrs r0, cpsr/* 传printException的第一个实参 */
	ldr r1, =swi_string/* 传printException的第二个实参 */
	bl printException/* 如果是Nand启动，printException函数在4K之外的话，
						这里就会出错，优化办法：将相对跳转到do_und改为
						绝对跳转*/
	sub r0, r4, #4 /* lr - 4就是异常指令的地址，并传参给printSwiVal */
	bl printSwiVal
	/* 恢复现场 */
	ldmia sp!, {r0-r12, pc}^ /* ^符号会把spsr的值恢复到cpsr里 */

swi_string:
	.string "swi exception"

.align 4/* 为避免受前面字符串长度的影响，加这条语句可以保证后面一条指令的地址是4字节对齐 */
		

do_irq:/* 异常处理 */

	/* 执行到这里之前:
	 * 1. lr_irq保存被中断模式中的下一条即将执行的指令的地址
	 * 2. SPSR_irq保存被中断模式的CPSR
	 * 3. CPSR中的M4-M0被设置为10010, 进入到und模式
	 * 4. 跳到0x18的地方执行程序 
	 */

	
	/* 异常模式下的sp没有设置，得设置 */
	ldr sp, =0x33d00000

	/*在und异常处理函数中有可能会修改r0- r12，所以先保存起来 */
	/*lr - 4是异常处理完后的返回地址，也要保存 */
	sub lr, lr, #4/*Table 2-2 */
	stmdb sp!, {r0-r12, lr}
	
	/* 保存现场 */
	/* 处理und异常 */

	bl handl_irq_c/* 如果是Nand启动，printException函数在4K之外的话，
						这里就会出错，优化办法：将相对跳转到do_und改为
						绝对跳转*/

	/* 恢复现场 */
	ldmia sp!, {r0-r12, pc}^ /* ^符号会把spsr_irq的值恢复到cpsr里 */

		
reset:

	/* 关闭看门狗 */
	ldr r0, =0x53000000
	ldr r1, =0
	str r1, [r0]

	/* 设置MPLL, FCLK : HCLK : PCLK = 400m : 100m : 50m */
	/* LOCKTIME(0x4C000000) = 0xFFFFFFFF */
	ldr r0, =0x4C000000
	ldr r1, =0xFFFFFFFF
	str r1, [r0]

	/* CLKDIVN(0x4C000014) = 0X5, tFCLK:tHCLK:tPCLK = 1:4:8  */
	ldr r0, =0x4C000014
	ldr r1, =0x5
	str r1, [r0]

	/* 设置CPU工作于异步模式 */
	mrc p15,0,r0,c1,c0,0
	orr r0,r0,#0xc0000000   //R1_nF:OR:R1_iA
	mcr p15,0,r0,c1,c0,0

	/* 设置MPLLCON(0x4C000004) = (92<<12)|(1<<4)|(1<<0) 
	 *  m = MDIV+8 = 92+8=100
	 *  p = PDIV+2 = 1+2 = 3
	 *  s = SDIV = 1
	 *  FCLK = 2*m*Fin/(p*2^s) = 2*100*12/(3*2^1)=400M
	 */
	ldr r0, =0x4C000004
	ldr r1, =(92<<12)|(1<<4)|(1<<0)
	str r1, [r0]

	/* 一旦设置PLL, 就会锁定lock time直到PLL输出稳定
	 * 然后CPU工作于新的频率FCLK
	 */
	
	bl enable_icache

	/* 设置内存: sp 栈 */
	/* 分辨是nor/nand启动
	 * 写0到0地址, 再读出来
	 * 如果得到0, 表示0地址上的内容被修改了, 它对应ram, 这就是nand启动
	 * 否则就是nor启动
	 */
	mov r1, #0
	ldr r0, [r1] /* 读出原来的值备份 */
	str r1, [r1] /* 0->[0] */ 
	ldr r2, [r1] /* r2=[0] */
	cmp r1, r2   /* r1==r2? 如果相等表示是NAND启动 */
	ldr sp, =0x40000000+4096 /* 先假设是nor启动 */
	moveq sp, #4096  /* nand启动 */
	streq r0, [r1]   /* 恢复原来的值 */

	bl sdram_init	

	/* 重定位text, rodata, data段整个程序 */
	bl copy2sdram/* src, dest, len */
	
	bl clean_bss

	/* 复位之后, cpu处于svc模式
	 * 现在, 切换到usr模式
	 */
	mrs r0, cpsr      /* 读出cpsr */
	bic r0, r0, #0xf  /* 修改M4-M0为0b10000, 进入usr模式, 以便后面实验从usr模式切换到了 */
	bic r0, r0, #(1<<7)  /* 清除CPSR中的bit 7，使能中断 */
	msr cpsr, r0
	

/*重定位完之后，以下函数可能在4K之外，所以，要让
以下函数在sdram执行 */
	/* 设置 sp_usr */
	ldr sp, =0x33f00000
	ldr pc, =sdram/* 把链接地址赋给pc */
	
sdram:
	bl uart0_init
	
	bl print1

	/* 故意设置一条未定义指令 */
und_code:
	.word 0xdeadc0de  /* 未定义指令，运行到这里就会发生异常 */
	bl print2

	swi 0x123 /* 执行此命令，触发SWI异常，进入0x8执行    */

	

	//bl main       /* 使用BL命令相对跳转, 程序仍然在NOR/sram执行 */
	ldr pc, =main  /* 绝对跳转, 跳到SDRAM */

halt:
	b halt

enable_icache:
	/* 设置协处理器C15的bit12为1，使能icache */
	mrc p15, 0, r0, c1, c0, 0
	orr r0, r0, #(1<<12)  /* r0 = r0 or (1<<12) */
	mcr p15, 0, r0, c1, c0, 0	
	mov pc, lr

