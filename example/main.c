/**
 ******************************************************************************
 * @file    main.c
 * @author  Ac6
 * @version V1.0
 * @date    01-December-2013
 * @brief   Default main function.
 ******************************************************************************
 */


#include "stm32f4xx.h"
#include "stm32f4_discovery.h"

#include "cmsis_os.h"
#include "queue.h"



N2OS_Queue(queueMsgt4_t5, 10*sizeof(uint32_t));
N2OS_Queue(queueMsgt1_t2, 10*sizeof(uint32_t));
N2OS_Queue(queueMsgt2_t3, 10*sizeof(uint32_t));

N2OS_Task(task1, 40, 4, 40)
{
	uint32_t outcoming_msg = 1;
	N2OS_begin_periodic
	{
		BSP_LED_Toggle(LED4);
		N2OS_queue_op_status_t status = enqueue(queueMsgt1_t2.queue_handler,
												  (uint8_t*)&outcoming_msg,
												  sizeof(uint32_t),
												  N2OS_NONBLOCKING_QUEUE_OP);
		if(N2OS_QUEUE_SUCCESS_OP == status)
			outcoming_msg=1-outcoming_msg;
	}
	N2OS_end_periodic;
}

N2OS_Task(task2, 40, 3, 500)
{
	uint32_t incoming_msg;
	uint32_t outcoming_msg = 1;
	N2OS_begin_periodic
	{
		N2OS_queue_op_status_t status = dequeue(
											queueMsgt1_t2.queue_handler,
											(uint8_t*)&incoming_msg,
											sizeof(uint32_t),
											N2OS_NONBLOCKING_QUEUE_OP);
		if(N2OS_QUEUE_SUCCESS_OP == status && incoming_msg == 1)
		{
			BSP_LED_Toggle(LED5);
			outcoming_msg = 1-outcoming_msg;
			enqueue(queueMsgt2_t3.queue_handler, (uint8_t*)&outcoming_msg, sizeof(uint32_t), N2OS_NONBLOCKING_QUEUE_OP);
		}
	}
	N2OS_end_periodic;
}


N2OS_Task(task3, 40, 2, 500)
{
	uint32_t incoming_msg;
	N2OS_begin_periodic
	{
		N2OS_queue_op_status_t status = dequeue(
										    queueMsgt2_t3.queue_handler,
											(uint8_t*)&incoming_msg,
											sizeof(uint32_t),
											N2OS_NONBLOCKING_QUEUE_OP);
		if(N2OS_QUEUE_SUCCESS_OP == status && incoming_msg == 1)
		BSP_LED_Toggle(LED6);
	}
	N2OS_end_periodic;
}


N2OS_Task(task4, 40, 99, 50)
{
	uint32_t msg = 0;
	N2OS_begin_periodic
	{
		if(GPIO_PIN_SET == BSP_PB_GetState(BUTTON_KEY))
		{
			msg = 1-msg;
			while(N2OS_QUEUE_SUCCESS_OP != enqueue(queueMsgt4_t5.queue_handler, (uint8_t*) & msg, sizeof(uint32_t), N2OS_NONBLOCKING_QUEUE_OP))
				osDelay(50);
			while(GPIO_PIN_SET == BSP_PB_GetState(BUTTON_KEY));
		}
	}
	N2OS_end_periodic;
}


N2OS_Task(task5, 40, 1, 50)
{
	uint32_t msg;
	N2OS_begin_periodic
	{
		dequeue(queueMsgt4_t5.queue_handler, (uint8_t*) & msg, sizeof(uint32_t), N2OS_NONBLOCKING_QUEUE_OP);
		if(msg == 1)
			BSP_LED_On(LED3);
		else
			BSP_LED_Off(LED3);
	}
	N2OS_end_periodic;
}

int main(void)
{
	HAL_Init();

	BSP_LED_Init(LED3);
	BSP_LED_Init(LED4);
	BSP_LED_Init(LED5);
	BSP_LED_Init(LED6);
	BSP_PB_Init(BUTTON_KEY, BUTTON_MODE_GPIO);

	osKernelInitialize();

	osThreadCreate(&task1, NULL);
	osThreadCreate(&task2, NULL);
	osThreadCreate(&task3, NULL);
	osThreadCreate(&task4, NULL);
	osThreadCreate(&task5, NULL);

	N2OS_create_queue_config(&queueMsgt1_t2);
	N2OS_create_queue_config(&queueMsgt2_t3);
	N2OS_create_queue_config(&queueMsgt4_t5);

	BSP_LED_Off(LED3);

	NVIC_SetPriority(PendSV_IRQn, 0xff); /* Lowest possible priority */
	NVIC_SetPriority(SysTick_IRQn, 0x00); /* Highest possible priority */

	osKernelStart();

	//The program should never reach this point
	Error_Handler();
	for(;;);
}

void Error_Handler()
{
	BSP_LED_On(LED3);
	for(;;);
}
