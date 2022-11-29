
/*
 *点亮灯D10，nLED1：GPF4
 */

.text
.global _start
_start:
/*
 * 配置GPF4为输出引脚
 * 把0x100写到地址0x56000050
 */

	ldr r1, =0x56000050
	ldr r0, =0x100
	str r0, [r1]

/*
 * 配置GPF4为输出为低电平
 * 把0写到地址0x56000054
 */

	ldr r1, =0x56000054
	ldr r0, =0
	str r0, [r1]

halt:
	b halt






 
