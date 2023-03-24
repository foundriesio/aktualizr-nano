/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_NET_H__
#define __AKNANO_NET_H__

#include "aknano.h"
#include "aknano_net_platform.h"

int init_network_context(struct aknano_network_context *network_context);

BaseType_t aknano_mtls_connect(struct aknano_network_context *network_context, const char *hostname, size_t hostname_len, uint16_t port, const char *server_root_ca, size_t server_root_ca_len);

BaseType_t aknano_mtls_send_http_request(struct aknano_network_context *network_context, const char *hostname, size_t hostname_len, const char *pcMethod, const char *pcPath, const char *pcBody, size_t xBodyLen, unsigned char *buffer, size_t buffer_len, const char **header_keys, const char **header_values, size_t header_len, int request_range_start, int request_range_end);

void aknano_mtls_disconnect(struct aknano_network_context *network_context);

void aknano_get_ipv4_and_mac(char *ipv4, uint8_t *mac);

#endif
