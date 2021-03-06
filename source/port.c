/**
 *
 * WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW
 * WOoloollokNMMMMMWOololloxXN0d''''''d0XNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
 * No''''''',dNMMMMNo'''''':0X''kkkko'''d0MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
 * No'''''''',dXMMMNo'''''':0WWWMMW0'''kKNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
 * No''''''''',dXMMNo'''''':0MMMWK'''MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
 * No'''''''''',oXMNo'''''':0MWKd'''XWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
 * No''''''''''',oXXo'''''':0Nx:':dOO000OOO0000OO0000000000000000000000OO00KXWMM
 * No'''''''''''',lOl'''''':0K''''''''''''''''''''''''''''''''''''''''''''''bONM
 * No'''''''''''''',,'''''':0XkXXXXXXXXXXXXXXXXXXXXXdxxOKOXXXXXXXXXXXXXXXXXXXXk0
 * No'''''''''''''''''''''':0O:'''''''''''''''''''''''d0l''''''''''''''''''''''c
 * No'''''''''''''''''''''':0O;'''''''''''''''''''''''o0c''''''''''''''''''''''c
 * No'''''''''''''''''''''':0O:'''''''''''''''''''''''o0c''''''''''''''''''''''c
 * No'''''''''''''''''''''':0O:'''''';looooool:'''''''oKo,'''''''''''''',cooooox
 * No'''''',:;''''''''''''':0O:''''''oNWWWWWMWx,''''''oNXkc,'''''''''''',o0WWMWM
 * No'''''':OO:'''''''''''':0O:''''''oNMMMMMMWx,''''''oNMMNOl,'''''''''''';dKWMM
 * No'''''':0W0c''''''''''':0O:''''''oNMMMMMMWx,''''''oNMMMMNOl,'''''''''''';dKW
 * No'''''':0MW0c'''''''''':0O:'''''':xkkkkkkxc'''''''oX0kkkkkd;'''''''''''''';x
 * No'''''':0MMW0c''''''''':0O:'''''''''''''''''''''''d0l''''''''''''''''''''''c
 * No'''''':0MMMMKc'''''''':0O:'''''''''''''''''''''''o0c''''''''''''''''''''''c
 * No'''''':0MMMMWKl''''''':0O;'''''''''''''''''''''''o0c''''''''''''''''''''''c
 * W0xxxxxxkXMMMMMMXkxxxxxxkXXkxxxxxxxxxxxxxxxxxxxxxxx0NOxxxxxxxxxxxxxxxxxxxxxxO
 * MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
 * *****************************************************************************
 * ************************* Give a new name to your OS ************************
 * *****************************************************************************
 *
 ******************************************************************************
 *
 * @file 	port.c
 * @author 	Mario Barbareschi <mario.barbareschi@unina.it>
 * @version V0.1
 * @date    13-November-2019
 * @brief 	Implementation file for function of port.h, in particular for
 * 			supporting ARM Cortex M3/M4 uC.
 *
 ******************************************************************************
 * @attention
 *
 *  Copyright (C) 2019  Mario Barbareschi
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************
 */


#include <N2OS_config.h>
#include "port.h"


void loadFirstTask()
{
	__asm volatile(	//" ldr r0,=_estack 		\n" //Take the original master stack pointer from the system startup
			" ldr R0,=0xE000ED08 	\n" //Set R0 to VTOR address
			" ldr r0, [r0] 			\n" //Load VTOR
			" ldr r0, [r0] 			\n" //Load initial MSP value
			" msr msp, r0			\n" // Set the msp back to the start of the stack
			" cpsie i				\n" //enable interrupt for getting the SWI 0
			" dsb					\n" //No instruction in program order after this instruction executes until this instruction completes
			" isb					\n" //It ensures that the effects of context altering operations,\
										  such as changing the ASID, or completed TLB maintenance operations,\
										  or branch predictor maintenance operations
			" svc 0					\n" // we invoke a SWI to start first task.
			" nop					\n"
			" .align 2				\n"

	);

}

void SVC_Handler()
{
	__asm volatile(
			" ldr	r2,=N2OS_curr_task 		\n"
			" ldr r1, [r2]						\n"
			" ldr r0, [r1] 						\n"//Get the stack pointer of the task
			" ldmia r0!, {r4-r11}				\n" // Pop registers that will be not automatically loaded on exception entry
			" msr psp, r0						\n"
			" mov r0, #0						\n" //Set 0 to basepri
			" msr	basepri, r0					\n"
			//Note that remaining registers are going to be automatically restored by returning from the ISR
			/* EXC_RETURN - Thread mode with PSP: */
			" orr r14, #0xd						\n"
			" bx r14							\n"
			" .align 2							\n"

	);
}

uint32_t* initialize_Stack(uint32_t* stack_ptr, void (*task_handler)(void), void (*task_completion)(void))
{
	/* The stack pointer is currently pointing to its lower position.
	 * Some register must be configured as they will be restored on exception return, namely:
   	 *  - XPSR: Default value (0x01000000)
   	 *  - PC: Point to the handler function
   	 *  - LR: Point to a function to be called when the handler returns
   	 *  Other general registers can be restored, even though we left them uninitialized.
   	 *  Consequently, at the end of this function, the stack pointer must grow of 64 bytes
   	 */
	*(stack_ptr) = 0x01000000;
	stack_ptr--;
	*(stack_ptr) = (uint32_t)task_handler;
	stack_ptr--;
	*(stack_ptr) = (uint32_t)task_completion;
	return stack_ptr-(16-3);
	/*
	+------+
	|      | <- SP now (64 bytes far from lower bound)
	|  R4  | empty
	|  R5  | empty
	|  R6  | empty
	|  R7  | empty
	|  R8  | empty
	|  R9  | empty
	| R10  | empty
	| R11  | empty
	|  R1  | empty
	|  R2  | empty
	|  R3  | empty
	| R12  | empty
	|  LR  | task_completion
	|  PC  | task_handler
	| xPSR | 0x01000000
	|      | <- SP outbound
	+------+
	*/
}

__attribute__(( naked )) void PendSV_Handler()
{
	__asm volatile(	/* Disable interrupts: */
			" cpsid	i 					\n"
			//Since entering this handler is due to NVIC, first 4 regs have been already pushed onto the stack
			/*
			+------+
			|      | <- SP after entering interrupt (orig. SP - 32 bytes)
			|  R0  |
			|  R1  |
			|  R2  |
			|  R3  |
			|  R12 |
			|  LR  | (R14)
			|  PC  | (Return address)
			| xPSR |
			|      | <- SP before interrupt (orig. SP)
			+------+

			 */

			" mrs		r0, psp 			\n" //Get the current stack pointer
			" dsb							\n"
			" isb							\n" //Flushes pipe
			" stmdb	r0!,{r4-r11} 			\n" //Save the remaining registers

			/*
			+------+
			|      | <- SP right now (orig. - 64 bytes)
			|  R4  |
			|  R5  |
			|  R6  |
			|  R7  |
			|  R8  |
			|  R9  |
			| R10  |
			| R11  |
			|  R1  |
			|  R2  |
			|  R3  |
			| R12  |
			|  LR  | (R14)
			|  PC  | (Return address)
			| xPSR |
			|      | <- SP before interrupt (orig. SP)
			+------+
			 */
			/* Save current task's SP: */
			" ldr	r2, =N2OS_curr_task 	\n"
			" ldr	r1, [r2] 				\n"
			" str	r0, [r1] 				\n"

			//Now we are ready to load the new context overwriting the one into the CPU

			/* Load next task's SP: */
			" ldr	r2, =N2OS_next_task 	\n"
			" ldr	r1, [r2] 				\n"
			" ldr	r0, [r1] 				\n"//Got the stack pointer

			//The process is pretty much like the same, but we pull instead
			" ldmia	r0!,{r4-r11} 			\n"
			" msr	psp, r0 				\n"
			//Note that remaining registers are going to be automatically restored by returning from the ISR
			/* EXC_RETURN - Thread mode with PSP: */
			" orr r14, #0xd					\n"
			/* Enable interrupts: */
			" cpsie	i	 					\n"
			" dsb							\n"
			" isb							\n"
			" bx	r14 					\n"
			" .align 2						\n"
	);
}


void yield()
{
	SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
	__DSB(); //__asm volatile( "dsb" );
	__ISB(); //__asm volatile( "isb" );
}

void enter_critical_section()
{
	__disable_irq();
}
void exit_critical_sction()
{
	__enable_irq();
}

int configureSystick(uint32_t systick_ticks)
{
	uint32_t ret_val = SysTick_Config(systick_ticks);
	//0  Function succeeded.
	//1  Function failed.
	if (ret_val == 1)
		return -1;
	return 0;
}
