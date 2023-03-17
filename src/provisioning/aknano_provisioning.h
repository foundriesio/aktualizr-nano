/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_PROVISIONING_H__
#define __AKNANO_PROVISIONING_H__

#include <stdint.h>

#ifdef AKNANO_ALLOW_PROVISIONING
void vDevModeKeyProvisioning_AkNano(uint8_t *client_key, uint8_t *client_certificate);
int aknano_provision_device();
#endif

#endif
