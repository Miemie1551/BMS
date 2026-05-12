#include "TaskInit.h"
#include "cmsis_os.h"

#include "DataAcqTask.h"

osThreadId_t dataAcqTaskHandle;
const osThreadAttr_t dataAcqTask_attributes = {
    .name = "dataAcqTask",
    .stack_size = 128 * 4,
    .priority = (osPriority_t)osPriorityNormal5,
};

void TaskInit(void)
{
    dataAcqTaskHandle = osThreadNew(DataAcqTask, NULL, &dataAcqTask_attributes);
}
