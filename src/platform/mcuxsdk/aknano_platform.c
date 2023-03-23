/*
 * Copyright 2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include "FreeRTOS.h"
#include "task.h"

void aknano_delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}
