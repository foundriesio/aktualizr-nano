/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_CLIENT_H__
#define __AKNANO_CLIENT_H__
#include "aknano_public_api.h"

/*
 * Functions that should be implemented by the client application
 */

time_t aknano_cli_get_current_epoch();
status_t aknano_cli_gen_random_bytes(char *output, size_t size);

#endif
