/*
 * Copyright 2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_BOARD_H__
#define __AKNANO_BOARD_H__

#include <stdint.h>

#include <aknano_compat_platform.h>

const char *aknano_get_board_name();

void aknano_delay(uint32_t ms);

void aknano_reboot_command();

#endif
