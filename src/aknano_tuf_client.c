/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include <string.h>

#include "core_json.h"

#include "aknano_debug.h"
#include "aknano_net.h"
#include "aknano_flash_storage.h"
#include "aknano_device_gateway.h"
#include "aknano.h"
#include "libtufnano.h"

#define JSON_ARRAY_LIMIT_COUNT 10

/* TUF "callbacks" */
static int get_flash_offset_for_role(enum tuf_role role)
{
    switch (role) {
    case ROLE_ROOT: return AKNANO_FLASH_OFF_TUF_ROLE_ROOT;
    case ROLE_TIMESTAMP: return AKNANO_FLASH_OFF_TUF_ROLE_TIMESTAMP;
    case ROLE_SNAPSHOT: return AKNANO_FLASH_OFF_TUF_ROLE_SNAPSHOT;
    case ROLE_TARGETS: return AKNANO_FLASH_OFF_TUF_ROLE_TARGETS;
    default: return -1;
    }
}

static int read_local_json_file(int initial_offset, unsigned char *target_buffer, size_t target_buffer_len, size_t *file_size)
{
    int ret;

    ret = aknano_read_flash_storage(initial_offset, target_buffer, target_buffer_len);
    if (ret < 0)
        return ret;

    if (target_buffer[0] != '{')
        return -1;

    for (size_t i = 0; i < target_buffer_len; i++) {
        if (target_buffer[i] == 0xFF) {
            target_buffer[i] = 0;
            break;
        }
    }

    target_buffer[target_buffer_len - 1] = 0;
    *file_size = strnlen((char *)target_buffer, target_buffer_len);
    return TUF_SUCCESS;
}

int tuf_client_read_local_file(enum tuf_role role, unsigned char *target_buffer, size_t target_buffer_len, size_t *file_size, void *application_context)
{
    int ret;
    int initial_offset;

    initial_offset = get_flash_offset_for_role(role);
    if (initial_offset < 0) {
        LogError((ANSI_COLOR_MAGENTA "tuf_client_read_local_file: role=%s error reading flash" ANSI_COLOR_RESET,
                  tuf_get_role_name(role)));
        return -1;
    }

    ret = read_local_json_file(initial_offset, target_buffer, target_buffer_len, file_size);
    if (ret < 0) {
        if (role == ROLE_ROOT) {
            ret = read_local_json_file(AKNANO_FLASH_OFF_TUF_ROOT_PROVISIONING, target_buffer, target_buffer_len, file_size);
            if (ret == TUF_SUCCESS)
                tuf_client_write_local_file(role, target_buffer, *file_size, application_context);
            return ret;
        }
        LogInfo((ANSI_COLOR_MAGENTA "tuf_client_read_local_file: role=%s file not found. buf[0]=%X" ANSI_COLOR_RESET,
                 tuf_get_role_name(role), target_buffer[0]));
        return -1; // File not found / read error
    }

    LogInfo((ANSI_COLOR_MAGENTA "tuf_client_read_local_file: role=%s file_size=%lu strlen=%lu OK" ANSI_COLOR_RESET,
             tuf_get_role_name(role), *file_size, strlen((const char *)target_buffer)));
    return TUF_SUCCESS;
}

int tuf_client_write_local_file(enum tuf_role role, const unsigned char *data, size_t len, void *application_context)
{
    (void)application_context;

    // int i;
    int initial_offset;
    status_t ret;

    initial_offset = get_flash_offset_for_role(role);
    // LogInfo(("write_local_file: role=%d initial_offset=%d len=%d", role, initial_offset, len));
    ret = aknano_write_data_to_storage(initial_offset, data, len);
    LogInfo((ANSI_COLOR_MAGENTA "tuf_client_write_local_file: role=%s len=%lu %s" ANSI_COLOR_RESET,
             tuf_get_role_name(role), len, ret? "ERROR" : "OK"));

    return ret;
}

int tuf_client_fetch_file(const char *file_base_name, unsigned char *target_buffer, size_t target_buffer_len, size_t *file_size, void *application_context)
{
    struct aknano_context *aknano_context = application_context;
    BaseType_t ret;

    *file_size = 0;
    snprintf((char *)aknano_context->url_buffer, sizeof(aknano_context->url_buffer), "/repo/%s", file_base_name);
    ret = aknano_send_http_request(
        aknano_context->dg_network_context,
        HTTP_METHOD_GET,
        (char *)aknano_context->url_buffer, "", 0,
        aknano_context->settings);

    if (ret == pdPASS) {
        LogInfo((ANSI_COLOR_MAGENTA "tuf_client_fetch_file: %s HTTP operation return code %d. Body length=%ld" ANSI_COLOR_RESET,
                 file_base_name, aknano_context->dg_network_context->reply_http_code, aknano_context->dg_network_context->reply_body_len));
        if ((aknano_context->dg_network_context->reply_http_code / 100) == 2) {
            if (aknano_context->dg_network_context->reply_body_len > target_buffer_len) {
                LogError(("tuf_client_fetch_file: %s retrieved file is too big. Maximum %ld, got %ld",
                          file_base_name, target_buffer_len, aknano_context->dg_network_context->reply_body_len));
                return TUF_ERROR_DATA_EXCEEDS_BUFFER_SIZE;
            }
            *file_size = aknano_context->dg_network_context->reply_body_len;
            /* aknano_context->dg_network_context->reply_body may overlap with target_buffer */
            memmove(target_buffer, aknano_context->dg_network_context->reply_body, aknano_context->dg_network_context->reply_body_len);
            target_buffer[aknano_context->dg_network_context->reply_body_len] = '\0';
            return TUF_SUCCESS;
        } else {
            return -aknano_context->dg_network_context->reply_http_code;
        }
    } else {
        LogInfo(("tuf_client_fetch_file: %s HTTP operation failed", file_base_name));
        return -1;
    }
}

#ifdef AKNANO_ALLOW_PROVISIONING
int aknano_provision_tuf_root(struct aknano_context *aknano_context)
{
    size_t file_size;
    int ret;
    const char *root_file_name = "1.root.json";

    if (read_local_json_file(AKNANO_FLASH_OFF_TUF_ROOT_PROVISIONING, ucUserBuffer, sizeof(ucUserBuffer), &file_size) == TUF_SUCCESS) {
        LogInfo(("aknano_provision_tuf_root: root json already provisioned"));
        return TUF_SUCCESS;
    }

    ret = tuf_client_fetch_file(root_file_name, ucUserBuffer, sizeof(ucUserBuffer), &file_size, aknano_context);
    LogInfo(("aknano_provision_tuf_root: fetch_file  ret=%d file_size=%ld", root_file_name, ret, file_size));

    if (ret == 0) {
        if (ucUserBuffer[0] != '{' || file_size < 100)
            return -1;

        // LogInfo(("write_local_file: role=%d initial_offset=%d len=%d", role, initial_offset, len));
        ret = aknano_write_data_to_storage(AKNANO_FLASH_OFF_TUF_ROOT_PROVISIONING, ucUserBuffer, file_size);
    }
    return ret;
}
#endif

#ifdef AKNANO_DELETE_PROVISIONED_TUF_ROOT
int aknano_clear_provisioned_tuf_root()
{
    return aknano_write_data_to_storage(AKNANO_FLASH_OFF_TUF_ROOT_PROVISIONING, "\xFF", 1);
}
#endif
