/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_DEBUG_H__
#define __AKNANO_DEBUG_H__

#ifndef LIBRARY_LOG_LEVEL
#define LIBRARY_LOG_LEVEL LOG_INFO
#endif

#ifndef LIBRARY_LOG_NAME
#define LIBRARY_LOG_NAME "aknano"
#endif

#include "logging_stack.h"

/*
 * Generic ANSI colors escape codes, used in console output
 */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

void aknano_dump_memory_info(const char *context);
#endif
