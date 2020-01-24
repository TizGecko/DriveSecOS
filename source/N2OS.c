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
 * @file 	N2OS.c
 * @author 	Mario Barbareschi <mario.barbareschi@unina.it>
 * @version V0.1
 * @date    13-November-2019
 * @brief 	Implementation file of main functionalities of N2OS.
 *		  	This file provides the system's tasks list structure, the N2OS
 *		  	ticks variable, two pointers that are used for the context switch by
 *		  	porting function and configured each time the scheduler is invoked.
 *		  	Additionally, this file contains the implementation of the idle task
 *		  	and the function executed by completed task.
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

#include <N2OS.h>

/**
 * Here we define a list containing all tasks that the scheduler must handle.
 * Additionally, the table reports the number of created tasks and the current
 * active task (there is one and only one active task at time)
 */
static struct
{
	N2OS_task_control_block_t tasks[N2OS_CONFIG_MAX_TASKS];
	volatile N2OS_task_id_t current_active_task; /** The current executing task */
	uint16_t size; /** Number of tasks that have been created */
} N2OS_tasks_list;

/**
 * This variable handles the N2OS ticks from the RTOS boot.
 */
volatile uint32_t N2OS_ticks;

/**
 * These two variables are used to get trace of current executing task
 * and one that has just been scheduled as next one. Once the yield
 * accomplishes, these variables are equal to each other.
 */
N2OS_task_control_block_t* volatile N2OS_curr_task;
N2OS_task_control_block_t* volatile N2OS_next_task;

/**
 * This values reports, in percentage, the execution of idle
 * during the scheduling lifetime. The value is update each time idle is
 * scheduled in a different N2OS time slice window.
 */
volatile uint8_t N2OS_idle_value = 0;

/**
 * N2OS_idle is the system idle task. It should be modified accordingly to
 * the system's needs.
 */
static void N2OS_idle()
{
	volatile uint64_t idleCount = 0;
	volatile uint64_t max_idleCount = 83;
	uint32_t last_N2OS_tick = 0;
	while (1){
		idleCount++;
		if(last_N2OS_tick != N2OS_ticks){
			N2OS_idle_value = idleCount*100/(max_idleCount*(N2OS_ticks-last_N2OS_tick));
			if(max_idleCount < idleCount / (N2OS_ticks-last_N2OS_tick))
				max_idleCount = idleCount / (N2OS_ticks-last_N2OS_tick);
			last_N2OS_tick = N2OS_ticks;
			idleCount = 0;
		}
		__disable_irq(); //Here the yield must be protected against other incoming interrupts
		N2OS_task_yield();
		__enable_irq();
	}
}

/**
 * The task_completion acts like a trap for those tasks that completes their
 * job. It should be modified accordingly to the system's needs.
 */
static void task_completion(void)
{
	/* This function is called when some task handler returns. */
	volatile uint32_t i = 0;
	while (1)
		i++;
}

static N2OS_stack_t idle_stack[N2OS_IDLE_TASK_STACK] __attribute__ ((aligned (4)));


void N2OS_init(void)
{
	memset(&N2OS_tasks_list, 0, sizeof(N2OS_tasks_list));
	N2OS_ticks = 0;
	/**
	 * Current idle process implementation does not need a lot of space,
	 * even though its minimum size depends on the target architecture.
	 */
	N2OS_task_init(N2OS_idle, idle_stack, N2OS_IDLE_TASK_STACK+1, 0, 0);
}

N2OS_task_id_t N2OS_task_init_config(N2OS_task_config_t config){
	return N2OS_task_init(config.handler, config.stack_ptr, config.stacksize, config.priority, config.period);
}

N2OS_task_id_t N2OS_task_init(void (*handler)(void), N2OS_stack_t* stack_ptr, uint32_t stack_size, N2OS_task_priority_t priority, N2OS_task_period_t period)
{
	if (N2OS_tasks_list.size >= N2OS_CONFIG_MAX_TASKS-1)
		return -1;

	/* Initialize the task structure and set SP to the top of the stack
	   minus 16 words (64 bytes) to leave space for storing 16 registers: */
	N2OS_task_control_block_t *p_task = &N2OS_tasks_list.tasks[N2OS_tasks_list.size];
	p_task->handler = handler;
	if(N2OS_MINIMUM_TASK_STACK_SIZE > stack_size)
		return -2;
		//stack_size = N2OS_MINIMUM_TASK_STACK_SIZE;
	//N2OS_stack_t *p_stack = malloc(stack_size*sizeof(N2OS_stack_t));
	N2OS_stack_t *p_stack = stack_ptr;
	p_task->status = N2OS_TASK_STATUS_READY;
	p_task->wait_ticks = 0;
	p_task->priority = priority;
	p_task->period = N2OS_CONFIG_SYSTICK_FREQ_DIV*period/1000;

	//Here we push the stack to it's lower limit, preparing it for the initialization
	p_stack += stack_size-1;

	p_task->sp = initialize_Stack(p_stack, handler, task_completion);
	N2OS_tasks_list.size++;

	/** The taskID of tasks starts from 0, while N2OS_idle does not have any ID, even though its index is 0 */
	return (N2OS_tasks_list.size-1);
}

int N2OS_start(uint32_t systick_ticks)
{
	configureSystick(systick_ticks);

	/* Start the first task: should be the first non-idle */
	N2OS_curr_task = &N2OS_tasks_list.tasks[N2OS_tasks_list.current_active_task];

	loadFirstTask();
	/** This point should be never reached */
	return 0;
}

void N2OS_systick_handler(void)
{
	++N2OS_ticks;

	int i;
	for(i = 1; i < N2OS_tasks_list.size; i++) //loop over tasks except for idle one
	{	//Remove tasks from suspend state if "wait_ticks" elapses
		//Does not matter N2OSTicks overflow, since we are checking the equality
		if(N2OS_TASK_STATUS_WAIT == N2OS_tasks_list.tasks[i].status && N2OS_tasks_list.tasks[i].wait_ticks == N2OS_ticks){
			N2OS_tasks_list.tasks[i].status = N2OS_TASK_STATUS_READY;
			N2OS_tasks_list.tasks[i].wait_ticks = 0;
		}
	}
	N2OS_task_yield();
}

void N2OS_task_yield(void)
{
	N2OS_scheduler();
	N2OS_next_task->last_active_time = N2OS_ticks;
	if(N2OS_next_task != N2OS_curr_task)
	{
		yield(); /** Actually, we are triggering the activation of the PendSV_Handler() */
	}
}

/**
 * N2OS scheduler makes use of this function for picking a task. If another algorithm
 * has to be executed, a different function must be provided and called
 */
void round_robin_scheduler(){
	int i = 0;
	do{
		++i;
		N2OS_tasks_list.current_active_task++;
		if (N2OS_tasks_list.current_active_task >= N2OS_tasks_list.size)
			N2OS_tasks_list.current_active_task = 1; //let's skip idle

	} while(i < N2OS_tasks_list.size && N2OS_TASK_STATUS_READY != N2OS_tasks_list.tasks[N2OS_tasks_list.current_active_task].status);
	if( i == N2OS_tasks_list.size ) //if we cycled over the task table, the only suitable task is the idle
	{
		N2OS_tasks_list.current_active_task = 0;
	}
}

void priority_scheduler()
{
	int i;
	//Let's save the task ID which has to be replaced
	//This information is useful to check if such a task is hanging the CPU
	N2OS_task_id_t last_active_task = N2OS_tasks_list.current_active_task;

	// Let's bet idle is the ready task with the highest priority
	N2OS_task_id_t task_to_be_load = 0;

	//Now, we need to loop overall tasks of the system
	for(i = 1; i < N2OS_tasks_list.size; i++)
	{
		if(N2OS_TASK_STATUS_READY == N2OS_tasks_list.tasks[i].status) // If the i-th task is ready
		{
			if(N2OS_tasks_list.tasks[i].priority > N2OS_tasks_list.tasks[task_to_be_load].priority) // If its priority is greater than the previous picked
			{
				if(last_active_task == i && //And the candidate is not hanging the CPU and not missed its own deadline
				  (N2OS_ticks - N2OS_tasks_list.tasks[i].last_activation_time) < N2OS_tasks_list.tasks[i].period)
				{
					task_to_be_load = i;
				} else if(task_to_be_load == 0) //Otherwise, if it is the same task scheduled before or if it is
												//hanging the CPU, but we are seeking a task to substitute the idle
												//it is ok the same
				{
					task_to_be_load = i;
				}
			}
		}
	}

	//It is important to keep update the scheduled time, so the last time (in N2OS Tick) on which a task has been scheduled
	if(task_to_be_load != last_active_task)
	{
		N2OS_tasks_list.tasks[task_to_be_load].last_activation_time = N2OS_ticks;
	}

	N2OS_tasks_list.current_active_task = task_to_be_load;
}

void N2OS_scheduler(void)
{
	/** Retrieve the current active task*/
	N2OS_curr_task = &N2OS_tasks_list.tasks[N2OS_tasks_list.current_active_task];

	/** If it is active, namely it has been set neither in wait nor suspend, its status must be changed in ready */
	if(N2OS_TASK_STATUS_ACTIVE == N2OS_curr_task->status)
		N2OS_curr_task->status = N2OS_TASK_STATUS_READY;

	/** Now, we need to pick the next task: */
	N2OS_SCHEDULER_FUNCTION();

	N2OS_next_task = &N2OS_tasks_list.tasks[N2OS_tasks_list.current_active_task];
	N2OS_next_task->status = N2OS_TASK_STATUS_ACTIVE;
}

void N2OS_delay(uint32_t millisec){
	N2OS_active_after(N2OS_CONFIG_SYSTICK_FREQ_DIV*millisec/1000);
}

void N2OS_active_after(uint32_t ticks){
	if(0 != ticks)
	{
		enter_critical_section(); //Here the yield and scheduling must be protected against other incoming interrupts
		{
			N2OS_tasks_list.tasks[N2OS_tasks_list.current_active_task].status = N2OS_TASK_STATUS_WAIT;
			N2OS_tasks_list.tasks[N2OS_tasks_list.current_active_task].wait_ticks = N2OS_ticks+ticks;
			N2OS_task_yield();
		}
		exit_critical_sction();
	}
}

N2OS_task_id_t get_current_task_id(void)
{
	return N2OS_tasks_list.current_active_task;
}

void set_current_task_status(N2OS_task_status_t status)
{
	set_task_status(N2OS_tasks_list.current_active_task, status);
}

void set_task_status(N2OS_task_id_t task_id, N2OS_task_status_t status)
{
	N2OS_tasks_list.tasks[task_id].status = status;
}

N2OS_task_status_t get_task_status(N2OS_task_id_t task_id)
{
	return N2OS_tasks_list.tasks[task_id].status;
}

uint32_t get_current_task_period(void)
{
	return N2OS_tasks_list.tasks[N2OS_tasks_list.current_active_task].period;
}

N2OS_task_status_t get_current_task_status(void)
{
	return get_task_status(N2OS_tasks_list.current_active_task);
}

uint8_t get_idle_percentage(void){
	return N2OS_idle_value;
}
