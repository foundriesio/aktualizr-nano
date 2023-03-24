/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "aknano_debug.h"
#include "aknano_net.h"
#include "aknano_platform.h"
#include "aknano_secret.h"
#include "aknano.h"
#include "libtufnano.h"


int init_network_context(struct aknano_network_context *network_context)
{
    memset(network_context, 0, sizeof(*network_context));
    return 0;
}

BaseType_t aknano_mtls_connect(
    struct aknano_network_context *network_context,
    const char *                   hostname,
    size_t                         hostname_len,
    uint16_t                       port,
    const char *                   server_root_ca,
    size_t                         server_root_ca_len
    )
{
    LogInfo(("aknano_mtls_connect %s", network_context->source_path));
    BaseType_t xStatus = pdPASS;

    LogInfo(("TLS session established"));
    return xStatus;
}



BaseType_t aknano_mtls_send_http_request(
    struct aknano_network_context *network_context,
    const char *                   hostname,
    size_t                         hostname_len,
    const char *                   pcMethod,
    const char *                   pcPath,
    const char *                   pcBody,
    size_t                         xBodyLen,
    unsigned char *                buffer,
    size_t                         buffer_len,
    const char **                  header_keys,
    const char **                  header_values,
    size_t                         header_len,
    int                            request_range_start,
    int                            request_range_end
    )
{
    LogInfo(("%s %s", __FUNCTION__, network_context->source_path));

    /* Return value of this method. */
    BaseType_t xStatus = pdPASS;
    return xStatus;
}

void aknano_mtls_disconnect(struct aknano_network_context *network_context)
{
    LogInfo(("%s %s", __FUNCTION__, network_context->source_path));
}

void aknano_get_ipv4_and_mac(char *ipv4, uint8_t *mac)
{
    ipv4[0] = (char)192;
    ipv4[1] = (char)168;
    ipv4[2] = (char)0;
    ipv4[3] = (char)20;

    mac[0] = (char)80;
    mac[1] = (char)80;
    mac[2] = (char)80;
    mac[3] = (char)80;
    mac[4] = (char)80;
    mac[5] = (char)80;
}
