/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include <stddef.h>

#include "FreeRTOS.h"
#include "task.h"

#include "aknano_debug.h"

#ifdef AKNANO_DUMP_MEMORY_USAGE_INFO
void aknano_dump_memory_info(const char *context)
{
    LogInfo(("MEMORY (%s): Stack high watermark: %u.  Minimum free heap: %u",
             context, uxTaskGetStackHighWaterMark(NULL), xPortGetMinimumEverFreeHeapSize()));
}
#endif
