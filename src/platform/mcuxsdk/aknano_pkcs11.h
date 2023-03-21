/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_PKCS11_H__
#define __AKNANO_PKCS11_H__

#include "core_pkcs11.h"
#include "pkcs11t.h"

#ifdef AKNANO_RESET_DEVICE_ID
CK_RV prvDestroyDefaultCryptoObjects(void);
#endif

#if defined(AKNANO_ENABLE_EXPLICIT_REGISTRATION) || defined(AKNANO_ALLOW_PROVISIONING)
bool aknano_read_device_certificate(char *dst, size_t dst_size);
#endif

#endif
