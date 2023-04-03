/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_DEBUG_PLATFORM_H__
#define __AKNANO_DEBUG_PLATFORM_H__

#include <stdio.h>

#define LogError(X) do { printf X; printf("\r\n"); } while (0)
#define LogWarn(X) do { printf X; printf("\r\n"); } while (0)
#define LogInfo(X) do { printf X; printf("\r\n"); } while (0)
#define LogDebug(X) do { printf X; printf("\r\n"); } while (0)

#endif
