/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_DEVICE_GATEWAY_H__
#define __AKNANO_DEVICE_GATEWAY_H__

#include <stdint.h>

#include "aknano.h"

bool aknano_send_event(struct aknano_settings *aknano_settings, const char *event_type, int new_version, bool success);

BaseType_t aknano_send_http_request(struct aknano_network_context *network_context,
                                    const char *                   pcMethod,
                                    const char *                   pcPath,
                                    const char *                   pcBody,
                                    size_t                         xBodyLen,
                                    struct aknano_settings *       aknano_settings
                                    );

int aknano_gen_serial_and_uuid(char *uuid_string, char *serial_string);

BaseType_t aknano_connect_to_device_gateway(struct aknano_network_context *network_context);

#endif
