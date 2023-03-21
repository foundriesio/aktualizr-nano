/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_NET_H__
#define __AKNANO_NET_H__

#include "core_http_client.h"
#include "transport_secure_sockets.h"

#include "aknano.h"

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

int init_network_context(struct aknano_network_context *network_context);

BaseType_t aknano_mtls_connect(struct aknano_network_context *network_context, const char *hostname, size_t hostname_len, uint16_t port, const char *server_root_ca, size_t server_root_ca_len);

BaseType_t aknano_mtls_send_http_request(struct aknano_network_context *network_context, const char *hostname, size_t hostname_len, const char *pcMethod, const char *pcPath, const char *pcBody, size_t xBodyLen, unsigned char *buffer, size_t buffer_len, const char **header_keys, const char **header_values, size_t header_len, int request_range_start, int request_range_end);

void aknano_mtls_disconnect(struct aknano_network_context *network_context);

void aknano_get_ipv4_and_mac(char *ipv4, uint8_t *mac);


#endif
