/*
 * Copyright 2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_PLATFORM_H__
#define __AKNANO_PLATFORM_H__

#include <stdint.h>

#include "board.h"

#define AKNANO_BOARD_NAME BOARD_NAME

void aknano_delay(uint32_t ms);

#endif