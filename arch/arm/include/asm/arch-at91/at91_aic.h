/*
 * [origin: Linux kernel include/asm-arm/arch-at91/at91_pit.h]
 *
 * Copyright (C) 2007 Andrew Victor
 * Copyright (C) 2007 Atmel Corporation.
 *
 * Periodic Interval Timer (PIT) - System peripherals regsters.
 * Based on AT91SAM9261 datasheet revision D.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef AT91_AIC_H
#define AT91_AIC_H

typedef struct at91_aic {
	u32	smr[32];	/* 0x00 Mode Register */
	u32	svr[32];	/* 0x04 Status Register */
	u32	ivr;	/* 0x08 Periodic Interval Value Register */
	u32	fvr;	/* 0x0C Periodic Interval Image Register */
	u32 isr;
	u32 ipr;
	u32 imr;
	u32 cisr;
	u32	reserved0[2];
	u32 iecr;
	u32 idcr;
	u32 iccr;
	u32 iscr;
	u32 eoicr;
	u32 spu;
	u32 dcr;
  u32 reserved1[1];
  u32 ffer;      /**< \brief (Aic Offset: 0x140) Fast Forcing Enable Register */
  u32 ffdr;      /**< \brief (Aic Offset: 0x144) Fast Forcing Disable Register */
  u32 ffsr;      /**< \brief (Aic Offset: 0x148) Fast Forcing Status Register */
  u32 reserved2[38];
  u32 wpmr;      /**< \brief (Aic Offset: 0x1E4) Write Protect Mode Register */
  u32 wpsr;      /**< \brief (Aic Offset: 0x1E8) Write Protect Status Register */
} at91_aic_t;

#define  BSP_INT_AIC_BASE                   ((u32)0xFFFFF000U)

#define  BSP_INT_AIC_SMR(n)         (*(u32 *)(BSP_INT_AIC_BASE  + 0x0000 + 4 * (n)))
#define  BSP_INT_AIC_SVR(n)         (*(u32 *)(BSP_INT_AIC_BASE  + 0x0080 + 4 * (n)))
#define  BSP_INT_AIC_IVR            (*(u32 *)(BSP_INT_AIC_BASE  + 0x0100))
#define  BSP_INT_AIC_FVR            (*(u32 *)(BSP_INT_AIC_BASE  + 0x0104))
#define  BSP_INT_AIC_ISR            (*(u32 *)(BSP_INT_AIC_BASE  + 0x0108))
#define  BSP_INT_AIC_IMR           (*(u32 *)(BSP_INT_AIC_BASE  + 0x0110))
#define  BSP_INT_AIC_IECR           (*(u32 *)(BSP_INT_AIC_BASE  + 0x0120))
#define  BSP_INT_AIC_IDCR           (*(u32 *)(BSP_INT_AIC_BASE  + 0x0124))
#define  BSP_INT_AIC_ICCR           (*(u32 *)(BSP_INT_AIC_BASE  + 0x0128))
#define  BSP_INT_AIC_EOICR          (*(u32 *)(BSP_INT_AIC_BASE  + 0x0130))
#define  BSP_INT_AIC_SPU          (*(u32 *)(BSP_INT_AIC_BASE  + 0x0134))
#define  BSP_INT_AIC_DCR          (*(u32 *)(BSP_INT_AIC_BASE  + 0x0138))

#define BSP_IntClr(per_id)				(BSP_INT_AIC_ICCR	= (1<<per_id))
#define BSP_IntDis(per_id)				(BSP_INT_AIC_IDCR	= (1<<per_id))
#define BSP_IntDisAll()						(BSP_INT_AIC_IDCR = 0xFFFFFFFF)
#define BSP_IntEn(per_id)				  (BSP_INT_AIC_IECR	= (1<<per_id))

void irq_install_handler (u8     per_id,
                      		u8     int_prio,
                      		u8     int_src_type,
                      		CPU_FNCT_VOID  int_isr_fnct);
                      		
#endif
