
#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

void aknano_delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}
