/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define LIBRARY_LOG_LEVEL LOG_INFO
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>

#include "aknano_client.h"
#include "aknano_priv.h"
#include "aknano_client.h"
#include "aknano_secret.h"
#include "libtufnano.h"

#define AKNANO_DEVICE_GATEWAY_ENDPOINT AKNANO_FACTORY_UUID ".ota-lite.foundries.io"

#define AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN ((uint16_t)(sizeof(AKNANO_DEVICE_GATEWAY_ENDPOINT) - 1))

static const uint32_t akNanoDeviceGateway_ROOT_CERTIFICATE_PEM_LEN = sizeof(AKNANO_DEVICE_GATEWAY_CERTIFICATE);

static char bodyBuffer[500];


void UpdateSettingValue(const char *, int);

static void get_time_str(time_t boot_up_epoch, char *output)
{
    time_t current_epoch_sec = get_current_epoch(boot_up_epoch);
    struct tm *tm = gmtime(&current_epoch_sec);

    sprintf(output, "%04d-%02d-%02dT%02d:%02d:%02d.%03dZ",
            tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
            tm->tm_hour, tm->tm_min, tm->tm_sec,
            0);
}


#include "psa/crypto.h"
static void btox(char *xp, const char *bb, int n)
{
    const char xx[] = "0123456789ABCDEF";

    while (--n >= 0) xp[n] = xx[(bb[n >> 1] >> ((1 - (n & 1)) << 2)) & 0xF];
}

static void serial_string_to_uuid_string(const char *serial, char *uuid)
{
    const char *s;
    char *u;
    int i;

    s = serial;
    u = uuid;

    for (i = 0; i < 36; i++) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            *u = '-';
        } else {
            *u = *s;
            s++;
        }
        u++;
    }
}


int aknano_gen_serial_and_uuid(char *uuid_string, char *serial_string)
{
    char serial_bytes[16];

    aknano_gen_random_bytes(serial_bytes, sizeof(serial_bytes));
    btox(serial_string, serial_bytes, sizeof(serial_bytes) * 2);
    serial_string_to_uuid_string(serial_string, uuid_string);
    uuid_string[36] = '\0';
    serial_string[32] = '\0';
    // LogInfo(("uuid='%s', serial='%s'", uuid_string, serial_string));
    return 0;
}


static bool fill_event_payload(char *payload,
                               struct aknano_settings *aknano_settings,
                               const char *event_type,
                               int new_version, bool success)
{
    int old_version = aknano_settings->running_version;
    char details[200];
    char current_time_str[50];
    char *correlation_id = aknano_settings->ongoing_update_correlation_id;
    char target[sizeof(aknano_settings->hwid) + 15];
    char evt_uuid[AKNANO_MAX_UUID_LENGTH], _serial_string[AKNANO_MAX_SERIAL_LENGTH];
    char *success_string;

    if (success)
        success_string = "\"success\": true,";
    else
        success_string = "\"success\": false,";

    aknano_gen_serial_and_uuid(evt_uuid, _serial_string);

    if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_COMPLETED)) {
        old_version = aknano_settings->last_confirmed_version;
        new_version = aknano_settings->running_version;
    }
    snprintf(target, sizeof(target), "%s-v%d", aknano_settings->hwid, new_version);

    if (strnlen(correlation_id, AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH) == 0)
        snprintf(correlation_id, AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH, "%s-%s", target, aknano_settings->uuid);

    if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_APPLIED)) {
        snprintf(details, sizeof(details), "Updating from v%d to v%d tag: %s. Image written to flash. Rebooting.",
                 old_version, new_version, aknano_settings->tag);
    } else if (!strcmp(event_type, AKNANO_EVENT_INSTALLATION_COMPLETED)) {
        if (!success)
            snprintf(details, sizeof(details), "Rollback to v%d after failed update to v%d.",
                     old_version, aknano_settings->last_applied_version);
        else
            snprintf(details, sizeof(details), "Updated from v%d to v%d. Image confirmed. Running on %s slot.",
                     old_version, new_version, aknano_settings->image_position == 1? "PRIMARY" : "SECONDARY");
    } else {
        snprintf(details, sizeof(details), "Updating from v%d to v%d tag: %s.",
                 old_version, new_version, aknano_settings->tag);
    }

    get_time_str(aknano_settings->boot_up_epoch, current_time_str);

    LogInfo(("fill_event_payload: time=%s cor_id=%s uuid=%s", current_time_str, correlation_id, evt_uuid));

    snprintf(payload, 1000,
             "[{" \
             "\"id\": \"%s\"," \
             "\"deviceTime\": \"%s\"," \
             "\"eventType\": {" \
             "\"id\": \"%s\"," \
             "\"version\": 0" \
             "}," \
             "\"event\": {" \
             "\"correlationId\": \"%s\"," \
             "\"targetName\": \"%s\"," \
             "\"version\": \"%d\"," \
             "%s" \
             "\"details\": \"%s\"" \
             "}" \
             "}]",
             evt_uuid, current_time_str, event_type,
             correlation_id, target, new_version,
             success_string, details);
    LogInfo(("Event: %s %s %s", event_type, details, success_string));
    return true;
}

BaseType_t aknano_connect_to_device_gateway(struct aknano_network_context *network_context)
{
    BaseType_t ret;

    init_network_context(network_context);
    ret = aknano_mtls_connect(network_context,
                              AKNANO_DEVICE_GATEWAY_ENDPOINT,
                              AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN,
                              AKNANO_DEVICE_GATEWAY_PORT,
                              AKNANO_DEVICE_GATEWAY_CERTIFICATE,
                              akNanoDeviceGateway_ROOT_CERTIFICATE_PEM_LEN);

    // LogInfo(("prvConnectToServer Result: %d", xDemoStatus));
    if (ret != pdPASS) {
        /* Log error to indicate connection failure after all
         * reconnect attempts are over. */
        LogError(("Failed to connect to HTTP server"));
        return pdFAIL;
    }
    return pdPASS;
}

BaseType_t aknano_send_http_request(struct aknano_network_context *network_context,
                                    const char *                   pcMethod,
                                    const char *                   pcPath,
                                    const char *                   pcBody,
                                    size_t                         xBodyLen,
                                    struct aknano_settings *       aknano_settings
                                    )
{
    char *tag = aknano_settings->tag;
    int version = aknano_settings->running_version;
    char active_target[200];

    snprintf(active_target, sizeof(active_target), "%s-v%d", aknano_settings->hwid, version);

    const char *header_keys[] = { "x-ats-tags", "x-ats-target" };
    const char *header_values[] = { tag, active_target };

    BaseType_t ret = aknano_mtls_send_http_request(
        network_context,
        AKNANO_DEVICE_GATEWAY_ENDPOINT,
        AKNANO_DEVICE_GATEWAY_ENDPOINT_LEN,
        pcMethod,
        pcPath,
        pcBody,
        xBodyLen,
        ucUserBuffer,
        sizeof(ucUserBuffer),
        header_keys,
        header_values,
        2);

    return ret;
}


bool aknano_send_event(struct aknano_settings *aknano_settings,
                       const char *event_type,
                       int new_version, bool success)
{
    struct aknano_network_context network_context;

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    if (!aknano_settings->is_device_registered) {
        LogInfo(("AkNanoSendEvent: Device is not registered. Skipping send of event %s", event_type));
        return TRUE;
    }
#endif

    BaseType_t xDemoStatus = pdPASS;

    xDemoStatus = aknano_connect_to_device_gateway(&network_context);
    if (xDemoStatus != pdPASS)
        return TRUE;

    fill_event_payload(bodyBuffer, aknano_settings, event_type, new_version, success);

    LogInfo((ANSI_COLOR_YELLOW "Sending %s event" ANSI_COLOR_RESET,
             event_type));
    LogInfo(("Event payload: %.80s (...)", bodyBuffer));

    aknano_send_http_request(
        &network_context,
        HTTP_METHOD_POST,
        "/events",
        bodyBuffer,
        strlen(bodyBuffer),
        aknano_settings);


    /* Close the network connection.  */
    aknano_mtls_disconnect(&network_context);
    return TRUE;
}
