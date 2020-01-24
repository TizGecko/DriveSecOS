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
 * @file 	N2OS.h
 * @author 	Mario Barbareschi <mario.barbareschi@unina.it>
 * @version V0.1
 * @date    13-November-2019
 * @brief 	Header file of main functionalities of N2OS.
 *		  	This file provides the N2OS task status enum, the definition of the
 * 		  	task control block and public function for end users to manage the
 * 		  	RTOS.
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

#ifndef N2OS_H
#define N2OS_H

#include <N2OS_config.h>
#include "port.h"

#include <string.h> //memcpy
#include <stdlib.h> //malloc

/**
 * @brief N2OS defines the stack as a long word 64-bits long.
 * This way the stack is forced to be aligned to 4 and push
 * and pop can be performed on every byte-group.
 */
typedef uint32_t N2OS_stack_t;

/**
 * @brief N2OS task ID is defined as a 16-bit long unsigned integer.
 * This means that it does support 65535 different tasks
 */
typedef uint16_t N2OS_task_id_t;

/**
 * @brief N2OS period is defined as a 16-bit long unsigned integer and expresses
 * the amount of time period in milliseconds, up to 65.535 s.
 */
typedef uint16_t N2OS_task_period_t;

/**
 * @brief N2OS defines different priority levels.
 * Minimum priority is fixed to 0, while the maximum priority can be
 * conveniently configured by giving value to macro N2OS_MAXIMUM_PRIORITY
 */
typedef uint8_t N2OS_task_priority_t;

/**
 * @brief N2OS supports tasks defined as function pointer with no arguments.
 */
typedef void (*N2OS_task_handler_t)(void);

/**
 * @brief This struct can be used to provide information to the initialization
 * function of N2OS.
 */
typedef struct N2OS_task_config  {
	N2OS_task_handler_t         handler;    // start address of thread function
	N2OS_stack_t*               stack_ptr;   // stack pointer
	uint32_t               		stacksize;    // stack size requirements in bytes; 0 is default stack size
	N2OS_task_priority_t        priority;      // priority of the task
	N2OS_task_period_t          period;        // period of the task in milliseconds
} N2OS_task_config_t;

/**
 * @brief This macro simplifies operations to define a N2OS task.
 * It manages the definition of the task handler, its own stack
 * and the declaration of the task for the programmer
 * @note The end user must call osThreadCreate() over the assigned
 * TASKNAME before running the scheduler
 */
#define N2OS_Task(TASKNAME, STACK_SIZE, PRIORITY, PERIOD)	static void TASKNAME ## _handler(void);\
												  	  	  	static N2OS_stack_t TASKNAME ## _stack[STACK_SIZE] __attribute__ ((aligned (4)));\
														    osThreadDef_t TASKNAME = {\
															  	   TASKNAME ## _handler,\
																   TASKNAME ## _stack,\
																   STACK_SIZE,\
																   PRIORITY,\
																   PERIOD\
														    };\
														    static void TASKNAME ## _handler(void)

/**
 * @brief There two following macros can be suitably used during the task definition.
 * The programmer needs to include the task periodic code between such two macros.
 */
#define N2OS_begin_periodic do{

#define N2OS_end_periodic N2OS_active_after(get_current_task_period());\
							}while (1)

/**
 * @brief The N2OS_task_status_t is the enumerative type that N2OS exploits
 * for implemeting the scheduler. Actually, N2OS handles four different
 * task status.
 *
 * ::N2OS_TASK_STATUS_READY indicates that a task is ready to be scheduled,
 * hence the scheduling algorithm can take it into consideration.
 *
 * ::N2OS_TASK_STATUS_ACTIVE indicates that a taks is currently executing.
 * As one can figure out, one and only one task a time takes the active
 * status.
 *
 * ::N2OS_TASK_STATUS_WAIT means that a task performed the N2OS_delay
 * function for some ticks which have not yet been elapsing. In this
 * status, a task cannot be scheduled as active.
 *
 * ::N2OS_TASK_STATUS_SUSPEND means that a task voluntary performed a yield
 * due to a resource contest, such as sending/receiving to/from a queue.
 * In this status, a task cannot be scheduled as active.
 */
typedef enum
{
	N2OS_TASK_STATUS_READY = 0, 	/**< Task is ready to be scheduled as active 								*/
	N2OS_TASK_STATUS_ACTIVE = 1, 	/**< Task is currently being executed										*/
	N2OS_TASK_STATUS_WAIT = -1,	/**< Task invoked the delay function										*/
	N2OS_TASK_STATUS_SUSPEND = 2	/**< Task invoked yield because a resource is busy or it does not exist yet	*/
} N2OS_task_status_t;

/**
 * @brief This struct defines the N2OS task control block.
 * It contains the task stack pointer, the function pointer to the task
 * implementation, the task status, the last time it has been scheduled
 * as active, the number of ticks that the N2OS system tick has to
 * reach to switch its status from ::N2OS_TASK_STATUS_WAIT to
 * ::N2OS_TASK_STATUS_READY.
 *
 * @param  None
 * @retval None
 */
typedef struct
{
	/* The stack pointer (sp) has to be the first element as it is located
	   at the same address as the structure itself (which makes it possible
	   to locate it safely from assembly implementation of PendSV_Handler()).
	   The compiler might add padding between other structure elements. */
	volatile uint32_t sp;
	void (*handler)(void);
	volatile N2OS_task_status_t status;
	volatile uint32_t last_active_time;
	volatile uint32_t last_activation_time;
	volatile uint32_t wait_ticks;
	volatile N2OS_task_priority_t priority;
	volatile uint32_t period;
} N2OS_task_control_block_t;

/**
 * @brief This function initialize the N2OS_tasks_list struct and populate it
 * with the first task of the system, namely the idle one.
 * For this reason, idle takes the threadID 0.
 *
 * @param  None
 * @retval None
 */
void N2OS_init(void);

/**
 * @brief This function initialize a new entry in the tasks list.
 * Actually, N2OS defines a task as a C function with no input/output parameters
 * (properly, non returning/cyclic).The stack size has to be statically determined,
 * taking into account the number of nesting function calling and related
 * static-allocated data (which belongs to the task's stack).
 *
 * Status of each created task is set to N2OS_TASK_STATUS_READY, meant that
 * the scheduler may pick it to be the current executed task, setting it to the
 * status ::N2OS_TASK_STATUS_ACTIVE.
 *
 * N2OS tasks are defined with a priority, which can be potentially used
 * by the scheduling algorithm
 *
 * @param [in] handler is the function pointer
 * @param [in] stack_ptr is the stack pointer
 * @param [in] stack_size decides the size of the task's stack
 * @param [in] priority is the priority value assigned to the task
 * @param [in] period is the period value (in milliseconds) assigned to the task
 * @retval the ::N2OS_task_id_t assigned to the task instance
 */
N2OS_task_id_t N2OS_task_init(void (*handler)(void), N2OS_stack_t* stack_ptr, uint32_t stack_size, N2OS_task_priority_t priority, N2OS_task_period_t period);

/**
 * @brief This function initialize a new entry in the tasks list.
 * Actually, N2OS defines a task as a C function with no input/output parameters
 * (properly, non returning/cyclic).The stack size has to be statically determined,
 * taking into account the number of nesting function calling and related
 * static-allocated data (which belongs to the task's stack).
 *
 * Status of each created task is set to N2OS_TASK_STATUS_READY, meant that
 * the scheduler may pick it to be the current executed task, setting it to the
 * status ::N2OS_TASK_STATUS_ACTIVE.
 *
 * N2OS tasks are defined with a priority, which can be potentially used
 * by the scheduling algorithm
 *
 * @param [in] config is a struct that contans the task handler, its stack pointer, the size of the task's stack\
 * the priority value assigned to the task and the period value (in milliseconds) assigned to the task.
 * @retval the ::N2OS_task_id_t assigned to the task instance
 */
N2OS_task_id_t N2OS_task_init_config(N2OS_task_config_t config);


/**
 * @brief This function makes N2OS started, meant that the system's timer has to\\
 * be configured accordingly to the N2OS defined sytick and the first task\\
 * (the idle one) has to be loaded.
 * Depending on the system target, some other
 * configurations must be provided, especially those related to the interrupt
 * controller, interrupt priority and additional driver configuration.
 *
 * @note Even though this function is declared to return an integer value, it
 * should never return since the first task has to be scheduled before reaching
 * the end of the function.
 *
 * @param [in] systick_ticks is the number of last_active_time's ticks between two interruption
 * events
 * @retval None. The function is declared for returning an integer, but this function should never return
 */
int N2OS_start(uint32_t systick_ticks);

/**
 * @brief This function implements the N2OS's scheduler.
 * The main aim is to get the current active task status to ::N2OS_TASK_STATUS_READY
 * (N2OS_current_task), pick a task from the N2OS's tasks list and assign it to
 * the N2OS_next_task, which is then set as ::N2OS_TASK_STATUS_READY.
 *
 * Currently, the scheduler implementation is just a round robin, hence the
 * scheduler picks the task next to the current one in the N2OS tasks list.
 * Obviously, a more sophisticated implementation is feasible, but maybe the
 * N2OS_task_control_block_t should be enriched with further information.
 *
 * Most important aspect, which does not depend on specific implementation of
 * scheduler, is the existance of the N2OS_idle task. Indded, each time the
 * scheduler is not able to determine a task to be executed, it should pick
 * the N2OS_idle task instead. In other words, the N2OS_idle task is the
 * least priority task of the system, matching the general meaing of the
 * idle task.
 *
 * @param  None
 * @retval None
 */
void N2OS_scheduler(void);

/**
 * @brief This function implements the task yield, hence it gives to a task the
 * possibility of a self-preemption for letting the scheduler pick another
 * task. Hence, first, the function must call the scheduler, then it forces
 * the system to call the yield, which forces the PendSV_Handler() ISR whenever
 * the scheduler picks a task different to the current active one.
 *
 * @param  None
 * @retval None
 */
void N2OS_task_yield(void);

/**
 * @brief This function allows to switch the task status from ::N2OS_TASK_STATUS_ACTIVE
 * to the ::N2OS_TASK_STATUS_WAIT.
 * In particular, the function must record the time (expressed in N2OS ticks) on
 * which the task status has to be switch to ::N2OS_TASK_STATUS_READY. Until the task
 * status is ::N2OS_TASK_STATUS_WAIT, the scheduler cannot execute (N2OS does not
 * admit task status transition from ::N2OS_TASK_STATUS_WAIT to
 * ::N2OS_TASK_STATUS_ACTIVE).
 *
 * Once the task status transition from ::N2OS_TASK_STATUS_ACTIVE to
 * N2OS_TASK_STATUS_WAIT completed, the function calls N2OS_task_yield to
 * call the scheduler and perform a context switch.
 *
 * @param [in] ticks represents the N2OS ticks that the scheduler must wait before
 * taking into account the task when it is called.
 * @retval None
 */
void N2OS_delay(uint32_t ticks);

/**
 * @brief This function represents the core of N2OS.
 * As a common implementation along RTOS projects, the systick handler must take care
 * about main duties of the RTOS that must be performed periodically. A basic one
 * comprehend the #N2OS_ticks incrementing, the verification of which tasks that have
 * the status ::N2OS_TASK_STATUS_WAIT must be switched as ::N2OS_TASK_STATUS_READY
 * and, finally, the scheduler calling.
 *
 * @param  None
 * @retval None
 */
void N2OS_systick_handler(void);

/**
 * @brief This accessory function returns the N2OS_task_id of the current active task
 * @param  None
 * @retval N2OS_task_id of the current active task
 */
N2OS_task_id_t get_current_task_id(void);

/**
 * @brief This accessory function configure the status of the current active task
 *
 * @param [in] status is the new status of current active task
 * @retval None
 */
void set_current_task_status(N2OS_task_status_t status);

/**
 * @brief This accessory function configure the status of the a task
 *
 * @param [in] task_id is the id of the task to modify
 * @param [in] status is the new status of current active task
 * @retval None
 */
void set_task_status(N2OS_task_id_t task_id, N2OS_task_status_t status);

/**
 * @brief This accessory function returns the status of the current active task
 *
 * @warning Currently, N2OS admits one and only one status when the task is
 * running, namely ::N2OS_TASK_STATUS_ACTIVE. However, future extensions may
 * plan more than one active status.
 *
 * @param None
 * @return N2OS_task_status of the current active task
 */
N2OS_task_status_t get_current_task_status(void);

/**
 * @brief This accessory function returns the period of the current active task
 *
 * @param None
 * @return N2OS_task_status of the current active task
 */
uint32_t get_current_task_period(void);

/**
 * @brief This accessory function returns the status of a given task
 *
 * @param [in] task_id is the ID of the task
 * @return N2OS_task_status of the task_id task
 */
N2OS_task_status_t get_task_status(N2OS_task_id_t task_id);

/**
 * @brief This function returns, in percentage, the idle of the processor
 *
 * @param None
 * @return the idle of the processor, between 0 and 100
 */
uint8_t get_idle_percentage(void);


#endif
