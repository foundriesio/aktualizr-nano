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

#include "lwip/netifapi.h"
#include "netif/ethernet.h"
#include "transport_secure_sockets.h"

#include "aknano_debug.h"
#include "aknano_net.h"
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
    ServerInfo_t xServerInfo = { 0 };
    SocketsConfig_t xSocketsConfig = { 0 };
    BaseType_t xStatus = pdPASS;
    TransportSocketStatus_t xNetworkStatus = TRANSPORT_SOCKET_STATUS_SUCCESS;

    network_context->xNetworkContext.pParams = &network_context->secureSocketsTransportParams;

    /* Initializer server information. */
    xServerInfo.pHostName = hostname;
    xServerInfo.hostNameLength = hostname_len;
    xServerInfo.port = port;

    /* Configure credentials for TLS mutual authenticated session. */
    xSocketsConfig.enableTls = true;
    xSocketsConfig.pAlpnProtos = NULL;
    xSocketsConfig.maxFragmentLength = 0;
    xSocketsConfig.disableSni = false;
    xSocketsConfig.pRootCa = server_root_ca;
    xSocketsConfig.rootCaSize = server_root_ca_len;
    xSocketsConfig.sendTimeoutMs = 3000;
    xSocketsConfig.recvTimeoutMs = 3000;

    /* Establish a TLS session with the HTTP server. This example connects to
     * the HTTP server as specified in democonfigAWS_IOT_ENDPOINT and
     * democonfigAWS_HTTP_PORT in http_demo_mutual_auth_config.h. */
    LogInfo(("Establishing a TLS session to %.*s:%d.",
             (int)xServerInfo.hostNameLength,
             xServerInfo.pHostName,
             xServerInfo.port));

    /* Attempt to create a mutually authenticated TLS connection. */
    xNetworkStatus = SecureSocketsTransport_Connect(&network_context->xNetworkContext,
                                                    &xServerInfo,
                                                    &xSocketsConfig);

    if (xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS) {
        LogError(("Error connecting to %.*s:%d. result=%d",
                  (int)xServerInfo.hostNameLength,
                  xServerInfo.pHostName,
                  xServerInfo.port, xNetworkStatus));
        xStatus = pdFAIL;
    } else {
        /* Define the transport interface. */
        network_context->xTransportInterface.pNetworkContext = &network_context->xNetworkContext;
        network_context->xTransportInterface.send = SecureSocketsTransport_Send;
        network_context->xTransportInterface.recv = SecureSocketsTransport_Recv;
    }

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
    size_t                         request_range_start,
    size_t                         request_range_end
    )
{
    /* Return value of this method. */
    BaseType_t xStatus = pdPASS;

    /* Configurations of the initial request headers that are passed to
     * #HTTPClient_InitializeRequestHeaders. */
    HTTPRequestInfo_t xRequestInfo;
    /* Represents a response returned from an HTTP server. */
    // HTTPResponse_t xResponse;
    /* Represents header data that will be sent in an HTTP request. */
    HTTPRequestHeaders_t xRequestHeaders;

    /* Return value of all methods from the HTTP Client library API. */
    HTTPStatus_t xHTTPStatus = HTTPSuccess;

    configASSERT(pcMethod != NULL);
    configASSERT(pcPath != NULL);

    /* Initialize all HTTP Client library API structs to 0. */
    (void)memset(&xRequestInfo, 0, sizeof(xRequestInfo));
    (void)memset(&network_context->xResponse, 0, sizeof(network_context->xResponse));
    (void)memset(&xRequestHeaders, 0, sizeof(xRequestHeaders));

    network_context->xResponse.getTime = xTaskGetTickCount;

    /* Initialize the request object. */
    xRequestInfo.pHost = hostname;
    xRequestInfo.hostLen = hostname_len;
    xRequestInfo.pMethod = pcMethod;
    xRequestInfo.methodLen = strlen(pcMethod);
    xRequestInfo.pPath = pcPath;
    xRequestInfo.pathLen = strlen(pcPath);

    /* Set "Connection" HTTP header to "keep-alive" so that multiple requests
     * can be sent over the same established TCP connection. */
    xRequestInfo.reqFlags = HTTP_REQUEST_KEEP_ALIVE_FLAG;

    /* Set the buffer used for storing request headers. */
    xRequestHeaders.pBuffer = buffer;
    xRequestHeaders.bufferLen = buffer_len;

    xHTTPStatus = HTTPClient_InitializeRequestHeaders(&xRequestHeaders,
                                                      &xRequestInfo);

    if (header_keys && header_values) {
        for (int i = 0; i < header_len; i++) {
            HTTPClient_AddHeader(&xRequestHeaders, header_keys[i], strlen(header_keys[i]), header_values[i], strlen(header_values[i]));
            // LogInfo(("Adding header %s=%s", header_keys[i], header_values[i]));
        }
    }

    if (request_range_start > 0 && request_range_end > 0) {
        LogInfo(("Setting range header %d->%d", request_range_start, request_range_end));
        HTTPClient_AddRangeHeader(&xRequestHeaders,
                                  request_range_start,
                                  request_range_end);
    }

    if (xHTTPStatus == HTTPSuccess) {
        /* Initialize the response object. The same buffer used for storing
         * request headers is reused here. */
        network_context->xResponse.pBuffer = buffer;
        network_context->xResponse.bufferLen = buffer_len;

        /* Send the request and receive the response. */
        xHTTPStatus = HTTPClient_Send(&network_context->xTransportInterface,
                                      &xRequestHeaders,
                                      (uint8_t *)pcBody,
                                      xBodyLen,
                                      &network_context->xResponse,
                                      0);
    } else {
        LogError(("Failed to initialize HTTP request headers: Error=%s.",
                  HTTPClient_strerror(xHTTPStatus)));
    }

    if (xHTTPStatus == HTTPSuccess) {
        LogInfo(("Received HTTP response from %s %.*s%.*s. Status Code=%u",
                 pcMethod, (int)hostname_len, hostname,
                 (int)xRequestInfo.pathLen, xRequestInfo.pPath, network_context->xResponse.statusCode));
        if (network_context->xResponse.statusCode == 403) {
            LogInfo(("Response Body: '%.*s'",
                    (int)network_context->xResponse.bodyLen, network_context->xResponse.pBody));
        }
        LogDebug(("Response Headers:\n%.*s\n",
                  (int)network_context->xResponse.headersLen, network_context->xResponse.pHeaders));
        // LogInfo( ( "Status Code: %u",
        //             pxResponse->statusCode ) );
        LogDebug(("Response Body:\n%.*s",
                  (int)network_context->xResponse.bodyLen, network_context->xResponse.pBody));
    } else {
        LogError(("Failed to send HTTP %.*s request to %.*s%.*s: Error=%s.",
                  (int)xRequestInfo.methodLen, xRequestInfo.pMethod,
                  (int)hostname_len, hostname,
                  (int)xRequestInfo.pathLen, xRequestInfo.pPath,
                  HTTPClient_strerror(xHTTPStatus)));
    }

    if (xHTTPStatus != HTTPSuccess)
        xStatus = pdFAIL;

    network_context->reply_body = network_context->xResponse.pBody;
    network_context->reply_body_len = network_context->xResponse.bodyLen;
    network_context->reply_http_code = network_context->xResponse.statusCode;

    return xStatus;
}

void aknano_mtls_disconnect(struct aknano_network_context *network_context)
{
    TransportSocketStatus_t xNetworkStatus;

    xNetworkStatus = SecureSocketsTransport_Disconnect(&network_context->xNetworkContext);
    if (xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS)
        LogError(("aknano_send_event Disconnection error: %d", xNetworkStatus));
}

extern struct netif netif;
void aknano_get_ipv4_and_mac(uint8_t *ipv4, uint8_t *mac)
{
    memcpy(ipv4, (uint8_t *)&netif.ip_addr.addr, 4);
    memcpy(mac, netif.hwaddr, 6);
}
