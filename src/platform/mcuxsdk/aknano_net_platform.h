/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_NET_PLATFORM_H__
#define __AKNANO_NET_PLATFORM_H__

#include "core_http_client.h"
#include "transport_secure_sockets.h"

/**
 * @brief Each compilation unit that consumes the NetworkContext must define it.
 * It should contain a single pointer to the type of your desired transport.
 * When using multiple transports in the same compilation unit, define this
 * pointer as void *.
 *
 * @note Transport stacks are defined in amazon-freertos/libraries/abstractions/transport/secure_sockets/transport_secure_sockets.h.
 */
struct NetworkContext {
    SecureSocketsTransportParams_t *pParams;
};

struct aknano_network_context {
    /* Platform specific fields */
    TransportInterface_t           xTransportInterface;
    /* The network context for the transport layer interface. */
    NetworkContext_t               xNetworkContext;
    TransportSocketStatus_t        xNetworkStatus;
    // BaseType_t xIsConnectionEstablished = pdFALSE;
    SecureSocketsTransportParams_t secureSocketsTransportParams;
    HTTPResponse_t                 xResponse;

    /* Platform independent fields */
    const unsigned char *          reply_body;
    size_t                         reply_body_len;
    int                            reply_http_code;
};

#endif
