/*
 * Copyright 2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>
#include <unistd.h>

#include "aknano_debug.h"

void aknano_delay(uint32_t ms)
{
     usleep(ms * 1000);
}

void aknano_reboot_command()
{
    LogInfo(("Test code. Skipping reboot..."));
}

void UpdateSettingValue(const char* name, int value){} // move to flash_*.h
