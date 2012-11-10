/*
 * (C) Copyright 2006
 * Stefan Roese, DENX Software Engineering, sr@denx.de.
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Marius Groeger <mgroeger@sysgo.de>
 *
 * (C) Copyright 2002
 * Sysgo Real-Time Solutions, GmbH <www.elinos.com>
 * Alex Zuepke <azu@sysgo.de>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/arch/hardware.h>
#include <asm/arch/at91_aic.h>
#include <asm/arch/at91_pit.h>
#include <asm/arch/at91_pmc.h>
#include <asm/arch/clk.h>
#include <asm/arch/io.h>
#include <asm/proc-armv/ptrace.h>

/*
*********************************************************************************************************
*                                           EXCEPTION DEFINES
*********************************************************************************************************
*/

                                                 /* ARM exception IDs                                  */
#define  CPU_RELOCATE_ARM_EXCEPT_RESET                                                                    0x00
#define  CPU_RELOCATE_ARM_EXCEPT_UNDEF_INSTR                                                              0x01
#define  CPU_RELOCATE_ARM_EXCEPT_SWI                                                                      0x02
#define  CPU_RELOCATE_ARM_EXCEPT_PREFETCH_ABORT                                                           0x03
#define  CPU_RELOCATE_ARM_EXCEPT_DATA_ABORT                                                               0x04
#define  CPU_RELOCATE_ARM_EXCEPT_ADDR_ABORT                                                               0x05
#define  CPU_RELOCATE_ARM_EXCEPT_IRQ                                                                      0x06
#define  CPU_RELOCATE_ARM_EXCEPT_FIQ                                                                      0x07
#define  CPU_RELOCATE_ARM_EXCEPT_NBR                                                                      0x08
                                                 /* ARM exception vectors addresses                    */
#define  CPU_RELOCATE_ARM_EXCEPT_RESET_VECT_ADDR              (CPU_RELOCATE_ARM_EXCEPT_RESET          * 0x04 + 0x00)
#define  CPU_RELOCATE_ARM_EXCEPT_UNDEF_INSTR_VECT_ADDR        (CPU_RELOCATE_ARM_EXCEPT_UNDEF_INSTR    * 0x04 + 0x00)
#define  CPU_RELOCATE_ARM_EXCEPT_SWI_VECT_ADDR                (CPU_RELOCATE_ARM_EXCEPT_SWI            * 0x04 + 0x00)
#define  CPU_RELOCATE_ARM_EXCEPT_PREFETCH_ABORT_VECT_ADDR     (CPU_RELOCATE_ARM_EXCEPT_PREFETCH_ABORT * 0x04 + 0x00)
#define  CPU_RELOCATE_ARM_EXCEPT_DATA_ABORT_VECT_ADDR         (CPU_RELOCATE_ARM_EXCEPT_DATA_ABORT     * 0x04 + 0x00)
#define  CPU_RELOCATE_ARM_EXCEPT_ADDR_ABORT_VECT_ADDR         (CPU_RELOCATE_ARM_EXCEPT_ADDR_ABORT     * 0x04 + 0x00)
#define  CPU_RELOCATE_ARM_EXCEPT_IRQ_VECT_ADDR                (CPU_RELOCATE_ARM_EXCEPT_IRQ            * 0x04 + 0x00)
#define  CPU_RELOCATE_ARM_EXCEPT_FIQ_VECT_ADDR                (CPU_RELOCATE_ARM_EXCEPT_FIQ            * 0x04 + 0x00)

                                                 /* ARM exception handlers addresses                   */
#define  CPU_RELOCATE_ARM_EXCEPT_RESET_HANDLER_ADDR           (CPU_RELOCATE_ARM_EXCEPT_RESET          * 0x04 + 0x20)
#define  CPU_RELOCATE_ARM_EXCEPT_UNDEF_INSTR_HANDLER_ADDR     (CPU_RELOCATE_ARM_EXCEPT_UNDEF_INSTR    * 0x04 + 0x20)
#define  CPU_RELOCATE_ARM_EXCEPT_SWI_HANDLER_ADDR             (CPU_RELOCATE_ARM_EXCEPT_SWI            * 0x04 + 0x20)
#define  CPU_RELOCATE_ARM_EXCEPT_PREFETCH_ABORT_HANDLER_ADDR  (CPU_RELOCATE_ARM_EXCEPT_PREFETCH_ABORT * 0x04 + 0x20)
#define  CPU_RELOCATE_ARM_EXCEPT_DATA_ABORT_HANDLER_ADDR      (CPU_RELOCATE_ARM_EXCEPT_DATA_ABORT     * 0x04 + 0x20)
#define  CPU_RELOCATE_ARM_EXCEPT_ADDR_ABORT_HANDLER_ADDR      (CPU_RELOCATE_ARM_EXCEPT_ADDR_ABORT     * 0x04 + 0x20)
#define  CPU_RELOCATE_ARM_EXCEPT_IRQ_HANDLER_ADDR             (CPU_RELOCATE_ARM_EXCEPT_IRQ            * 0x04 + 0x20)
#define  CPU_RELOCATE_ARM_EXCEPT_FIQ_HANDLER_ADDR             (CPU_RELOCATE_ARM_EXCEPT_FIQ            * 0x04 + 0x20)

                                                 /* ARM "Jump To Self" asm instruction                 */
#define  CPU_RELOCATE_ARM_INSTR_JUMP_TO_SELF                   0xEAFFFFFE
                                                 /* ARM "Jump To Exception Handler" asm instruction    */
#define  CPU_RELOCATE_ARM_INSTR_JUMP_TO_HANDLER                0xE59FF018

void  BSP_IntSched(u8  int_type) ;

void       undefined_instruction   (void);
void       software_interrupt          (void);
void       prefetch_abort(void);
void       data_abort    (void);
void       not_used    (void);
void       irq          (void);
void       fiq          (void);

void  CPU_Relocate_InitExceptVect (void)
{
    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_UNDEF_INSTR_VECT_ADDR)       =         CPU_RELOCATE_ARM_INSTR_JUMP_TO_HANDLER;
    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_UNDEF_INSTR_HANDLER_ADDR)    = (u32)undefined_instruction;

    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_SWI_VECT_ADDR)               =         CPU_RELOCATE_ARM_INSTR_JUMP_TO_HANDLER;
    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_SWI_HANDLER_ADDR)            = (u32)software_interrupt;

    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_PREFETCH_ABORT_VECT_ADDR)    =         CPU_RELOCATE_ARM_INSTR_JUMP_TO_HANDLER;
    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_PREFETCH_ABORT_HANDLER_ADDR) = (u32)prefetch_abort;

    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_DATA_ABORT_VECT_ADDR)        =         CPU_RELOCATE_ARM_INSTR_JUMP_TO_HANDLER;
    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_DATA_ABORT_HANDLER_ADDR)     = (u32)data_abort;

    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_ADDR_ABORT_VECT_ADDR)        =         CPU_RELOCATE_ARM_INSTR_JUMP_TO_HANDLER;
    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_ADDR_ABORT_HANDLER_ADDR)     = (u32)not_used;

    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_IRQ_VECT_ADDR)               =         CPU_RELOCATE_ARM_INSTR_JUMP_TO_HANDLER;
    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_IRQ_HANDLER_ADDR)            = (u32)irq;

    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_FIQ_VECT_ADDR)               =         CPU_RELOCATE_ARM_INSTR_JUMP_TO_HANDLER;
    (*(u32 *)CPU_RELOCATE_ARM_EXCEPT_FIQ_HANDLER_ADDR)            = (u32)fiq;
}

static void default_isr(void)
{
	u32  irq_id;


	irq_id = (BSP_INT_AIC_ISR & 0x1F);                          /* Retrieve the ID of the interrupting source         */
	BSP_IntClr(irq_id);
	
	//printf("default_isr():  called for IRQ %d.\n", (int)irq_id);
}

void do_irq (struct pt_regs *pt_regs)
{
	BSP_IntSched(BSP_INT_CTRL_TYPE_IRQ);
}

void CSP_IntVectSet (u8     per_id,
										 u8     int_prio,
										 u8     int_src_type,
										 CPU_FNCT_VOID  int_isr_fnct)
{
    u8  aic_scr_type;
                      
                                                                /* ---------------- ARGUMENTS CHECKING -------------- */
    if (int_prio > BSP_INT_PRIO_HIGHEST) {
        return;
    }
    
    if (int_isr_fnct == (CPU_FNCT_VOID)0 ) {
        return;
    }
 
    if (per_id > AT91_ID_MAX) { 
        return;
    }

                                                                
    BSP_INT_AIC_SVR(per_id) = (u32)int_isr_fnct;         /* Set the interrupt vector with the ISR handler      */
    
    switch (int_src_type) {
        case BSP_INT_SCR_TYPE_INT_HIGH_LEVEL_SENSITIVE:
        case BSP_INT_SCR_TYPE_EXT_LOW_LEVEL_SENSITIVE:
             aic_scr_type = 0x00;
             break;
    
        case BSP_INT_SCR_TYPE_INT_POSITIVE_EDGE_TRIG:
        case BSP_INT_SCR_TYPE_EXT_NEGATIVE_EDGE_TRIG:
             aic_scr_type = 0x01;
             break;

        case BSP_INT_SCR_TYPE_EXT_HIGH_LEVEL_SENSITIVE:
             aic_scr_type = 0x02;
             break;

        case BSP_INT_SCR_TYPE_EXT_POSITIVE_EDGE_TRIG:
             aic_scr_type = 0x03;
             break;

        default:
             aic_scr_type = 0x00;
             break;             
    }
     
    BSP_INT_AIC_SMR(per_id) = (int_prio         )               /* Set Interrupt priority and type                    */
                            | (aic_scr_type << 5);
}

void  BSP_IntSched(u8  int_type) 
{
    CPU_FNCT_VOID   p_isr;
    
    if (int_type == BSP_INT_CTRL_TYPE_FIQ) {    
        p_isr = (CPU_FNCT_VOID)BSP_INT_AIC_FVR;                 /* Read the interrupt vector from the VIC             */        

        while (p_isr != (CPU_FNCT_VOID)0) {                     /* Make sure we don't have a NULL pointer             */

            (*p_isr)();
            BSP_INT_AIC_EOICR = 0;                   /* End of handler                                     */   
            p_isr = (CPU_FNCT_VOID)BSP_INT_AIC_FVR;             /* Read the interrupt vector from the VIC             */
    }
        BSP_INT_AIC_EOICR = 0;                       /* End of handler                                     */
    }

    if (int_type == BSP_INT_CTRL_TYPE_IRQ) {    
        p_isr = (CPU_FNCT_VOID)BSP_INT_AIC_IVR;                 /* Read the interrupt vector from the VIC             */        

        while (p_isr != (CPU_FNCT_VOID)0) {                     /* Make sure we don't have a NULL pointer             */

            (*p_isr)();
            BSP_INT_AIC_EOICR = 0;                   /* End of handler                                     */   
            p_isr = (CPU_FNCT_VOID)BSP_INT_AIC_IVR;             /* Read the interrupt vector from the VIC             */
        }
        BSP_INT_AIC_EOICR = 0;                       /* End of handler                                     */
    }
}

int arch_interrupt_init (void)
{
	u8  per_id;


	default_isr();
	BSP_INT_AIC_EOICR = 0;                           /* End-of-interrupt command                           */

                                                                /* Initialize all ISR's to the Dummy ISR handler      */
	for (per_id = AT91_ID_FIQ; per_id < AT91_ID_MAX; per_id++) {     
		CSP_IntVectSet((u8   )per_id,
												(u8   )BSP_INT_PRIO_LOWEST,
                        (u8   )BSP_INT_SCR_TYPE_INT_HIGH_LEVEL_SENSITIVE,
                        (CPU_FNCT_VOID)default_isr);
		BSP_INT_AIC_EOICR = 0;
	}
	CPU_Relocate_InitExceptVect();
	return (0);
}
