/*
 * Copyright 2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <time.h>
#include <stdio.h>
#include <stdint.h>

#include "mcuboot_app_support.h"
#include "FreeRTOS.h"
#include "task.h"

#include "aknano_debug.h"

void aknano_delay(uint32_t ms)
{
    vTaskDelay(pdMS_TO_TICKS(ms));
}

void aknano_reboot_command()
{
#ifdef AKNANO_DISABLE_REBOOT
    LogInfo(("Skipping reboot..."));
    LogInfo(("Halting task execution"));
    while (1)
        ;

#else
    LogInfo(("Rebooting...."));
    aknano_delay(100);
    NVIC_SystemReset();
#endif
}
