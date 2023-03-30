/*
 * Copyright 2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_PLATFORM_H__
#define __AKNANO_PLATFORM_H__

#include <stdint.h>

#include "mbedtls/compat-2.x.h"

typedef long BaseType_t;
#define pdFALSE                                  ( ( BaseType_t ) 0 )
#define pdTRUE                                   ( ( BaseType_t ) 1 )

#define pdPASS                                   ( pdTRUE )
#define pdFAIL                                   ( pdFALSE )


typedef int32_t status_t;

// TODO mock this value
#define AKNANO_BOARD_NAME "MIMXRT1060-EVK"

void aknano_delay(uint32_t ms);

#endif
