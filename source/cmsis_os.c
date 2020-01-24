#include "cmsis_os.h"

osStatus osKernelInitialize (void){
	N2OS_init();
}

osStatus osKernelStart (void)
{
  N2OS_start(N2OS_CONFIG_SYSTICK_FREQ);
  return osOK;
}

uint32_t osKernelSysTick(void)
{
	N2OS_systick_handler();
}

osThreadId osThreadCreate (const osThreadDef_t *thread_def, void *argument)
{
	return N2OS_task_init(thread_def->pthread, thread_def->stack_ptr, thread_def->stacksize, thread_def->priority, thread_def->period);
  
}

osThreadId osThreadGetId (void)
{
	return NULL;
}

osStatus osThreadTerminate (osThreadId thread_id)
{
  return osErrorOS;
}

osStatus osThreadYield (void)
{
	N2OS_task_yield();
  return osOK;
}

osStatus osThreadSetPriority (osThreadId thread_id, osPriority priority)
{
  return osErrorOS;
}


osPriority osThreadGetPriority (osThreadId thread_id)
{
  return osPriorityError;
}

osStatus osDelay (uint32_t millisec)
{
	N2OS_delay(millisec);
	return osOK;
}

