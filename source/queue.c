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
 * @file 	queue.c
 * @author 	Mario Barbareschi <mario.barbareschi@unina.it>
 * @version V0.1
 * @date    13-November-2019
 * @brief 	Implementation file of N2OS queue. It just contains
 * 			implementation of function declared in the corresponding header file
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

#include "queue.h"

N2OS_queue* createQueue(uint8_t* buffer, unsigned int size)
{
	N2OS_queue* queue = (N2OS_queue*) malloc(sizeof(N2OS_queue));
	queue->size = size;
	queue->queueMemory = buffer;
	//queue->queueMemory = malloc(size);
	queue->rLock = N2OS_QUEUE_UNLOCKED; /** we assume to create an unlocked queue */
	queue->wLock = N2OS_QUEUE_UNLOCKED;
	reset_queue(queue); /** reset_queue() is used to perform remaining initializations */
	return queue;
}

N2OS_queue* N2OS_create_queue_config(N2OS_queue_config_t* config){
	config->queue_handler = createQueue(config->buffer, config->buffer_size);
	return config->queue_handler;
}

N2OS_queue_op_status_t enqueue(N2OS_queue* queue, uint8_t* msg, unsigned int msg_size, N2OS_blocking_queue_op_t blocking)
{
	int writtenFlag = 0;
	while(0 == writtenFlag) //This flag will be set once the writing is achieved
	{
		enter_critical_section();
		{
			if(N2OS_QUEUE_UNLOCKED == queue->wLock)
			{
				queue->wLock = N2OS_QUEUE_LOCKED;
				if(msg_size > queue->freeMemory) //Check available space and yield if no enough space
				{
					if(N2OS_BLOCKING_QUEUE_OP == blocking)
					{
						queue->tasks_waiting_to_send[get_current_task_id()] = 1;
						set_current_task_status(N2OS_TASK_STATUS_SUSPEND);
						N2OS_task_yield(); /** the yield call has no effect since it is invoked inside a critical section!
						 	 	 	 	 	  *	It will eventually have effect once the critical section ends.
						 	 	 	 	 	  */
					}
					else
					{ /**
						* Since we are executing a non-blocking enqueue, we return from this function signaling that
						* the queue is full (::N2OS_QUEUE_FULL_OP). Before returning, we must exit from the critical
						* section as well as we must unlock the writing operations on the queue
					    */
						queue->wLock = N2OS_QUEUE_UNLOCKED;
						exit_critical_sction();
						return N2OS_QUEUE_FULL_OP;
					}
				}
				else //Space available
				{
					if(queue->head+msg_size <= queue->size) //The message just is append
					{
						memcpy(queue->queueMemory+queue->head, msg, msg_size);
						queue->head += msg_size;
					}
					else //We need to split the copy
					{
						memcpy(queue->queueMemory+queue->head, msg, queue->size-queue->head);
						memcpy(queue->queueMemory, msg+queue->size-queue->head, msg_size-(queue->size-queue->head));
						queue->head = msg_size-(queue->size-queue->head); //The head just point to the first free location
					}
					queue->freeMemory -= msg_size;
					//Let's make awaken suspended tasks
					int i;
					for(i = 0; i < N2OS_CONFIG_MAX_TASKS; i++)
						if(1 == queue->tasks_waiting_to_receive[i])
						{
							set_task_status(i, N2OS_TASK_STATUS_READY);
							queue->tasks_waiting_to_receive[i] = 0;
						}
					writtenFlag = 1;
				}
				queue->wLock = N2OS_QUEUE_UNLOCKED;
			}
			else //The queue is just locked for sending
			{
				if(N2OS_BLOCKING_QUEUE_OP == blocking)
				{
					N2OS_task_yield();	/** The yield call has no effect since it is invoked inside a critical section!
										 	 *	It will eventually have effect once the critical section ends.
										 	 */
				}
				else
				{  /**
					* Since we are executing a non-blocking enqueue, we return from this function signaling that
					* the queue is busy (::N2OS_QUEUE_BUSY_OP). Before returning, we must exit from the critical
					* section
				    */
					exit_critical_sction();
					return N2OS_QUEUE_BUSY_OP;
				}
			}
		}
		exit_critical_sction();
	}
	/**
	 * Reaching this point implies that the while loop terminates upon the condition writtenFlag != 0,
	 * meaning that we successfully enqueued a message
	 */
	return N2OS_QUEUE_SUCCESS_OP;
}

N2OS_queue_op_status_t dequeue(N2OS_queue* queue, uint8_t* msg, unsigned int msg_size, N2OS_blocking_queue_op_t blocking)
{
	int receivedFlag = 0;
	while(0 == receivedFlag) //This flag will be set once the writing is achieved
	{
		enter_critical_section();
		{
			if(N2OS_QUEUE_UNLOCKED == queue->rLock)
			{
				queue->rLock = N2OS_QUEUE_LOCKED;
				if(msg_size > (queue->size - queue->freeMemory)) //Check whether the queue is containing at least one msg
				{
					if(N2OS_BLOCKING_QUEUE_OP == blocking)
					{
						queue->tasks_waiting_to_receive[get_current_task_id()] = 1;
						set_current_task_status(N2OS_TASK_STATUS_SUSPEND);
						N2OS_task_yield(); /** the yield call has no effect since it is invoked inside a critical section!
											  *	 It will eventually have effect once the critical section ends.
											  */
					}
					else
					{  /**
						* Since we are executing a non-blocking dequeue, we return from this function signaling that
						* the queue is empty (::N2OS_QUEUE_EMPTY_OP). Before returning, we must exit from the critical
						* section as well as we must unlock the reading operations on the queue
					    */
						queue->rLock = N2OS_QUEUE_UNLOCKED;
						exit_critical_sction();
						return N2OS_QUEUE_EMPTY_OP;
					}
				}
				else //There is at least one msg
				{
					if(queue->tail+msg_size <= queue->size) //If the message is stored along the queue
					{
						memcpy(msg, queue->queueMemory+queue->tail,msg_size);
						queue->tail += msg_size;
					}
					else //We need to split the copy
					{
						memcpy(msg, queue->queueMemory+queue->tail, queue->size-queue->tail);
						memcpy(msg+queue->size-queue->tail, queue->queueMemory, msg_size-(queue->size-queue->tail));
						queue->tail = msg_size-(queue->size-queue->tail); //The tail just point to the next non-free region
					}
					queue->freeMemory += msg_size;
					//Let's make awaken suspended tasks
					int i;
					for(i = 0; i < N2OS_CONFIG_MAX_TASKS; i++)
						if(1 == queue->tasks_waiting_to_send[i])
						{
							set_task_status(i, N2OS_TASK_STATUS_READY);
							queue->tasks_waiting_to_send[i] = 0;
						}
					receivedFlag = 1;
				}
				queue->rLock = N2OS_QUEUE_UNLOCKED;
			}
			else //The queue is just locked for receiving
			{
				if(N2OS_BLOCKING_QUEUE_OP == blocking)
				{
					N2OS_task_yield();	/** the yield call has no effect since it is invoked inside a critical section!
					 	 	 	 	 	 	 *	 It will eventually have effect once the critical section ends.
					 	 	 	 	 	 	 */
				}
				else
				{  /**
					* Since we are executing a non-blocking dequeue, we return from this function signaling that
					* the queue is busy (::N2OS_QUEUE_BUSY_OP). Before returning, we must exit from the critical
					* section.
				    */
					exit_critical_sction();
					return N2OS_QUEUE_BUSY_OP;
				}
			}
		}
		exit_critical_sction();
	}
	/**
	 * Reaching this point implies that the while loop terminates upon the condition receivedFlag != 0,
	 * meaning that we successfully dequeue a message
	 */
	return N2OS_QUEUE_SUCCESS_OP;
}

void reset_queue(N2OS_queue* queue)
{
	if(N2OS_QUEUE_UNLOCKED == queue->rLock && N2OS_QUEUE_UNLOCKED == queue->wLock) /** before continue, we must be sure the queue are not blocked */
	{
		int i;
		for(i = 0; i < queue->size; i++)
			queue->queueMemory[i] = 0;
		queue->head = 0;
		queue->tail = 0;
		queue->freeMemory = queue->size;
		for(i = 0; i < N2OS_CONFIG_MAX_TASKS; i++)
		{
			queue->tasks_waiting_to_receive[i] = 0;
			queue->tasks_waiting_to_send[i] = 0;
		}
	}
}
