.syntax unified
.cpu cortex-m0plus
.fpu softvfp
.thumb

.global g_pfnVectors
.global Default_Handler

.section .text.Reset_Handler
.weak Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
  bl  SystemInit

  ldr r0, =_sdata
  ldr r1, =_edata
  ldr r2, =_sidata
  movs r3, #0
  b LoopCopyDataInit

CopyDataInit:
  ldr r4, [r2, r3]
  str r4, [r0, r3]
  adds r3, r3, #4

LoopCopyDataInit:
  adds r4, r0, r3
  cmp r4, r1
  bcc CopyDataInit

  ldr r2, =_sbss
  ldr r4, =_ebss
  movs r3, #0
  b LoopFillZerobss

FillZerobss:
  str  r3, [r2]
  adds r2, r2, #4

LoopFillZerobss:
  cmp r2, r4
  bcc FillZerobss

  bl __libc_init_array
  bl main
  bx lr
  .ltorg
.size Reset_Handler, .-Reset_Handler

.section .text.Default_Handler,"ax",%progbits
.thumb_func
.type Default_Handler, %function
Default_Handler:
Infinite_Loop:
  b Infinite_Loop
.size Default_Handler, .-Default_Handler

.section .isr_vector,"a",%progbits
.type g_pfnVectors, %object
.size g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVC_Handler
  .word 0
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler
  .word WDT_IRQHandler
  .word LVD_IRQHandler
  .word RTC_IRQHandler
  .word FLASHRAM_IRQHandler
  .word SYSCTRL_IRQHandler
  .word GPIOA_IRQHandler
  .word GPIOB_IRQHandler
  .word GPIOC_IRQHandler
  .word GPIOF_IRQHandler
  .word DMACH12_IRQHandler
  .word DMACH34_IRQHandler
  .word CORDIC_IRQHandler
  .word ADC1_IRQHandler
  .word ATIM_IRQHandler
  .word VC13_IRQHandler
  .word VC24_IRQHandler
  .word GTIM1_IRQHandler
  .word GTIM2_IRQHandler
  .word GTIM34_IRQHandler
  .word LPTIM_IRQHandler
  .word BTIM1_IRQHandler
  .word BTIM2_IRQHandler
  .word BTIM3_HALLTIM_IRQHandler
  .word I2C1_IRQHandler
  .word I2C2_IRQHandler
  .word SPI1_IRQHandler
  .word SPI23_IRQHandler
  .word UART1_IRQHandler
  .word UART2_IRQHandler
  .word UART3_IRQHandler
  .word ADC2_DAC_IRQHandler
  .word CLKFAULT_IRQHandler

  .weak NMI_Handler
  .thumb_set NMI_Handler,Default_Handler

  .weak HardFault_Handler
  .thumb_set HardFault_Handler,Default_Handler

  .weak SVC_Handler
  .thumb_set SVC_Handler,Default_Handler

  .weak PendSV_Handler
  .thumb_set PendSV_Handler,Default_Handler

  .weak SysTick_Handler
  .thumb_set SysTick_Handler,Default_Handler

  .weak WDT_IRQHandler
  .thumb_set WDT_IRQHandler,Default_Handler

  .weak LVD_IRQHandler
  .thumb_set LVD_IRQHandler,Default_Handler

  .weak RTC_IRQHandler
  .thumb_set RTC_IRQHandler,Default_Handler

  .weak FLASHRAM_IRQHandler
  .thumb_set FLASHRAM_IRQHandler,Default_Handler

  .weak SYSCTRL_IRQHandler
  .thumb_set SYSCTRL_IRQHandler,Default_Handler

  .weak GPIOA_IRQHandler
  .thumb_set GPIOA_IRQHandler,Default_Handler

  .weak GPIOB_IRQHandler
  .thumb_set GPIOB_IRQHandler,Default_Handler

  .weak GPIOC_IRQHandler
  .thumb_set GPIOC_IRQHandler,Default_Handler

  .weak GPIOF_IRQHandler
  .thumb_set GPIOF_IRQHandler,Default_Handler

  .weak DMACH12_IRQHandler
  .thumb_set DMACH12_IRQHandler,Default_Handler

  .weak DMACH34_IRQHandler
  .thumb_set DMACH34_IRQHandler,Default_Handler

  .weak CORDIC_IRQHandler
  .thumb_set CORDIC_IRQHandler,Default_Handler

  .weak ADC1_IRQHandler
  .thumb_set ADC1_IRQHandler,Default_Handler

  .weak ATIM_IRQHandler
  .thumb_set ATIM_IRQHandler,Default_Handler

  .weak VC13_IRQHandler
  .thumb_set VC13_IRQHandler,Default_Handler

  .weak VC24_IRQHandler
  .thumb_set VC24_IRQHandler,Default_Handler

  .weak GTIM1_IRQHandler
  .thumb_set GTIM1_IRQHandler,Default_Handler

  .weak GTIM2_IRQHandler
  .thumb_set GTIM2_IRQHandler,Default_Handler

  .weak GTIM34_IRQHandler
  .thumb_set GTIM34_IRQHandler,Default_Handler

  .weak LPTIM_IRQHandler
  .thumb_set LPTIM_IRQHandler,Default_Handler

  .weak BTIM1_IRQHandler
  .thumb_set BTIM1_IRQHandler,Default_Handler

  .weak BTIM2_IRQHandler
  .thumb_set BTIM2_IRQHandler,Default_Handler

  .weak BTIM3_HALLTIM_IRQHandler
  .thumb_set BTIM3_HALLTIM_IRQHandler,Default_Handler

  .weak I2C1_IRQHandler
  .thumb_set I2C1_IRQHandler,Default_Handler

  .weak I2C2_IRQHandler
  .thumb_set I2C2_IRQHandler,Default_Handler

  .weak SPI1_IRQHandler
  .thumb_set SPI1_IRQHandler,Default_Handler

  .weak SPI23_IRQHandler
  .thumb_set SPI23_IRQHandler,Default_Handler

  .weak UART1_IRQHandler
  .thumb_set UART1_IRQHandler,Default_Handler

  .weak UART2_IRQHandler
  .thumb_set UART2_IRQHandler,Default_Handler

  .weak UART3_IRQHandler
  .thumb_set UART3_IRQHandler,Default_Handler

  .weak ADC2_DAC_IRQHandler
  .thumb_set ADC2_DAC_IRQHandler,Default_Handler

  .weak CLKFAULT_IRQHandler
  .thumb_set CLKFAULT_IRQHandler,Default_Handler
  