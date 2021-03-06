#ifndef IRD_H
#define IRQ_H

#define VIC1_BASE 0x800B0000
#define VIC2_BASE 0x800C0000

#define VICxIRQStatus	   0x00	//RO	One bit for each interrupt source
								              //		1 if interrupt is asserted and enabled
#define VICxFIQStatus	   0x04	//RO	As above for FIQ
#define VICxRawIntr		   0x08	//RO	As above but not masked
#define VICxIntSelect	   0x0c	//R/W	0: IRQ, 1: FIQ
#define VICxIntEnable	   0x10	//R/W	0: Masked, 1: Enabled
#define VICxIntEnClear   0x14	//WO	Clears bits in VICxIntEnable
#define VICxSoftInt	     0x18	//R/W	Asserts interrupt from software
#define VICxSoftIntClear 0x1c	//WO	Clears interrupt from software
#define VICxProtection	 0x20	//R/W	Bit 0 enables protection from user mode access
#define VICxVectAddr	   0x30	//R/W	Enables priority hardware


#define UART1_IntReceive  52
#define UART1_IntTransmit 24
#define UART2_IntReceive  25
#define UART2_IntTransmit 26

#define TC3UI 51

void irq_enable_timer(void);

void irq_clear_timer(void);

void irq_disable_timer(void);

void timer_notifier(void);

void irq_enable_uart1_receive(void);

void irq_enable_uart1_transmit(void);

void irq_enable_uart2_receive(void);

void irq_enable_uart2_transmit(void);

#endif /* IRQ_H */
