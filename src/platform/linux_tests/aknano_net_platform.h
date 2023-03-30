/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_NET_PLATFORM_H__
#define __AKNANO_NET_PLATFORM_H__

#include <stddef.h>

struct aknano_network_context {
    const char *source_path;
    bool is_connected;


    /* Platform independent fields */
    const unsigned char *          reply_body;
    size_t                         reply_body_len;
    int                            reply_http_code;
};

#define HTTP_METHOD_GET     "GET"                       /**< HTTP Method GET string. */
#define HTTP_METHOD_PUT     "PUT"                       /**< HTTP Method PUT string. */
#define HTTP_METHOD_POST    "POST"                      /**< HTTP Method POST string. */
#define HTTP_METHOD_HEAD    "HEAD"                      /**< HTTP Method HEAD string. */

#endif
