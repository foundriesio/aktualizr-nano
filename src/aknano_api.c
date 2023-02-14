/*
 * Copyright 2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define LIBRARY_LOG_LEVEL LOG_INFO

#include <time.h>
#include <stdio.h>

#include "lwip/opt.h"
#include "lwip/apps/sntp.h"
#include "sntp_example.h"
#include "lwip/netif.h"

#include "aknano_priv.h"
#include "aknano_secret.h"
#include "flexspi_flash_config.h"
#include "libtufnano.h"

static void fill_network_info(char *output, size_t max_length)
{
    char ipv4[4];
    uint8_t mac[6];

    aknano_get_ipv4_and_mac(ipv4, mac);

    snprintf(output, max_length,
             "{" \
             " \"local_ipv4\": \"%u.%u.%u.%u\"," \
             " \"mac\": \"%02x:%02x:%02x:%02x:%02x:%02x\"" \
             "}",
             ipv4[0], ipv4[1], ipv4[2], ipv4[3],
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]
             );

    LogInfo(("fill_network_info: %s", output));
}

static void parse_config(const char *config_data, int buffer_len, struct aknano_settings *aknano_settings)
{
    JSONStatus_t result = JSON_Validate(config_data, buffer_len);
    char *value;
    unsigned int valueLength;
    int int_value;

    if (result == JSONSuccess) {
        result = JSON_Search(config_data, buffer_len,
                             "tag" TUF_JSON_QUERY_KEY_SEPARATOR "Value", strlen("tag" TUF_JSON_QUERY_KEY_SEPARATOR "Value"),
                             &value, &valueLength);

        if (result == JSONSuccess) {
            if (strncmp(value, aknano_settings->tag, valueLength)) {
                LogInfo(("parse_config_data: Tag has changed ('%s' => '%.*s')",
                         aknano_settings->tag,
                         valueLength, value));
                strncpy(aknano_settings->tag, value,
                        valueLength);
                aknano_settings->tag[valueLength] = '\0';
            }
        } else {
            LogInfo(("parse_config_data: Tag config not found"));
        }

        result = JSON_Search(config_data, buffer_len,
                             "polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value", strlen("polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value"),
                             &value, &valueLength);

        if (result == JSONSuccess) {
            if (sscanf(value, "%d", &int_value) <= 0) {
                LogWarn(("Invalid polling_interval '%s'", value));
            } else {
                if (int_value != aknano_settings->polling_interval) {
                    LogInfo(("parse_config_data: Polling interval has changed (%d => %d)",
                             aknano_settings->polling_interval, int_value));
                    aknano_settings->polling_interval = int_value;
                }
            }
        } else {
            LogInfo(("parse_config_data: polling_interval config not found"));
        }

        result = JSON_Search(config_data, buffer_len,
                             "btn_polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value", strlen("btn_polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value"),
                             &value, &valueLength);

        if (result == JSONSuccess) {
            if (sscanf(value, "%d", &int_value) <= 0)
                LogWarn(("Invalid btn_polling_interval '%s'", value));
            else
                UpdateSettingValue("btn_polling_interval", int_value);
        }
    } else {
        LogWarn(("Invalid config JSON result=%d", result));
    }
}

void aknano_init_context(struct aknano_context * aknano_context,
                         struct aknano_settings *aknano_settings)
{
    memset(aknano_context, 0, sizeof(*aknano_context));
    aknano_context->settings = aknano_settings;
}

int aknano_checkin(struct aknano_context *aknano_context)
{
    static char bodyBuffer[200];

    struct aknano_network_context network_context;
    BaseType_t xDemoStatus;
    bool is_update_required = false;
    bool is_reboot_required = false;
    struct aknano_settings *aknano_settings = aknano_context->settings;
    int tuf_ret = 0;

#ifdef AKNANO_ALLOW_PROVISIONING
    static bool tuf_root_is_provisioned = false;
#endif
#ifdef AKNANO_DUMP_MEMORY_USAGE_INFO
    aknano_dump_memory_info("Before aknano_checkin");
#endif
    LogInfo(("aknano_checkin. Version=%lu  Tag=%s", aknano_settings->running_version, aknano_settings->tag));

    xDemoStatus = aknano_connect_to_device_gateway(&network_context);
    if (xDemoStatus == pdPASS) {
        xDemoStatus = aknano_send_http_request(
            &network_context,
            HTTP_METHOD_GET,
            "/config", "", 0,
            aknano_context->settings);
        if (xDemoStatus == pdPASS) {
            if (network_context.reply_body == NULL)
                LogInfo(("Device has no config set"));
            else
                parse_config((const char *)network_context.reply_body, network_context.reply_body_len, aknano_context->settings);
        }

        snprintf(bodyBuffer, sizeof(bodyBuffer),
                 "{ " \
                 " \"product\": \"%s\"," \
                 " \"description\": \"Aktualizr-nano PoC\"," \
                 " \"claimed\": true, " \
                 " \"serial\": \"%s\", " \
                 " \"id\": \"%s\", " \
                 " \"class\": \"MCU\" " \
                 "}",
                 AKNANO_BOARD_NAME, aknano_settings->serial, AKNANO_BOARD_NAME);

        aknano_send_http_request(&network_context, HTTP_METHOD_PUT,
                                 "/system_info", bodyBuffer, strlen(bodyBuffer),
                                 aknano_context->settings);

        fill_network_info(bodyBuffer, sizeof(bodyBuffer));
        aknano_send_http_request(&network_context, HTTP_METHOD_PUT,
                                 "/system_info/network", bodyBuffer, strlen(bodyBuffer),
                                 aknano_context->settings);

        // LogInfo(("aknano_settings->tag=%s",aknano_settings->tag));
        sprintf(bodyBuffer,
                "[aknano_settings]\n" \
                "poll_interval = %d\n" \
                "hw_id = \"%s\"\n" \
                "tag = \"%s\"\n" \
                "binary_compilation_local_time = \""__DATE__ " " __TIME__ "\"\n"
                "provisioning_mode = \"" AKNANO_PROVISIONING_MODE "\"",
                aknano_settings->polling_interval,
                AKNANO_BOARD_NAME,
                aknano_settings->tag);
        aknano_send_http_request(&network_context, HTTP_METHOD_PUT,
                                 "/system_info/config", bodyBuffer, strlen(bodyBuffer),
                                 aknano_context->settings);

        time_t reference_time = get_current_epoch(aknano_settings->boot_up_epoch);
// #define TUF_FORCE_DATE_IN_FUTURE 1
#ifdef TUF_FORCE_DATE_IN_FUTURE
        LogInfo((ANSI_COLOR_RED "Forcing TUF reference date to be 1 year from now" ANSI_COLOR_RESET));
        reference_time += 31536000; // Add 1 year
#endif
        aknano_context->dg_network_context = &network_context;

#ifdef AKNANO_ALLOW_PROVISIONING
        if (!tuf_root_is_provisioned) {
            tuf_ret = aknano_provision_tuf_root(aknano_context);
            tuf_root_is_provisioned = (tuf_ret == 0);
        }
#endif
        if (tuf_ret == 0) {
            tuf_ret = tuf_refresh(aknano_context, reference_time, tuf_data_buffer, sizeof(tuf_data_buffer));
            LogInfo((ANSI_COLOR_MAGENTA "tuf_refresh %s (%d)" ANSI_COLOR_RESET, tuf_get_error_string(tuf_ret), tuf_ret));
        }

        if (tuf_ret == 0)
            parse_targets_metadata((const char *)tuf_data_buffer, strlen((char *)tuf_data_buffer), aknano_context);
    }
    /* Close the network connection.  */
    aknano_mtls_disconnect(&network_context);
#ifdef AKNANO_DUMP_MEMORY_USAGE_INFO
    aknano_dump_memory_info("After aknano_checkin");
#endif
    return xDemoStatus == 1? 0 : -1;
}

bool aknano_install_selected_target(struct aknano_context *aknano_context)
{
    bool is_reboot_required = false;
    char unused_serial[AKNANO_MAX_SERIAL_LENGTH];
    struct aknano_settings *aknano_settings = aknano_context->settings;

    /* Gen a random correlation ID for the update events */
    aknano_gen_serial_and_uuid(aknano_settings->ongoing_update_correlation_id,
                               unused_serial);

    aknano_send_event(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_STARTED,
                      aknano_context->selected_target.version,
                      AKNANO_EVENT_SUCCESS_UNDEFINED);
    if (aknano_download_and_flash_image(aknano_context)) {
        aknano_send_event(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_COMPLETED,
                          aknano_context->selected_target.version,
                          AKNANO_EVENT_SUCCESS_TRUE);
        aknano_send_event(aknano_context->settings, AKNANO_EVENT_INSTALLATION_STARTED,
                          aknano_context->selected_target.version,
                          AKNANO_EVENT_SUCCESS_UNDEFINED);

        aknano_settings->last_applied_version = aknano_context->selected_target.version;
        aknano_send_event(aknano_context->settings, AKNANO_EVENT_INSTALLATION_APPLIED,
                          aknano_context->selected_target.version,
                          AKNANO_EVENT_SUCCESS_TRUE);

        LogInfo(("Requesting update on next boot (ReadyForTest)"));
        status_t status;
        status = bl_update_image_state(kSwapType_ReadyForTest);
        if (status != kStatus_Success)
            LogWarn(("Error setting image as ReadyForTest. status=%d", status));
        else
            is_reboot_required = true;
    } else {
        aknano_send_event(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_COMPLETED,
                          aknano_context->selected_target.version,
                          AKNANO_EVENT_SUCCESS_FALSE);
    }

    aknano_update_settings_in_flash(aknano_settings);
    return is_reboot_required;
}

bool aknano_has_matching_target(struct aknano_context *aknano_context)
{
    return aknano_context->selected_target.version != 0;
}

bool aknano_is_rollback(struct aknano_context *aknano_context)
{
    return aknano_context->selected_target.version == aknano_context->settings->last_applied_version;
}

int aknano_get_current(struct aknano_context *aknano_context)
{
    return aknano_context->settings->running_version;
}

int aknano_get_selected_version(struct aknano_context *aknano_context)
{
    return aknano_context->selected_target.version;
}

void aknano_reboot_command()
{
    LogInfo(("Rebooting...."));
    vTaskDelay(pdMS_TO_TICKS(100));
    NVIC_SystemReset();
}

int aknano_get_setting(struct aknano_context *aknano_context, const char *setting_name)
{
    if (!strncmp(setting_name, "polling_interval", strlen("polling_interval")))
        return aknano_context->settings->polling_interval;
    LogError(("Invalid setting name %s", setting_name));
    return 0;
}

int limit_sleep_time_range(int sleep_time)
{
    /* No less than 5 seconds, no more than 1 hour */
    if (sleep_time < 5)
        return 5;
    else if (sleep_time > 60 * 60)
        return 60 * 60;
    else
        return sleep_time;
}

void aknano_sample_loop()
{
    static struct aknano_settings aknano_settings;

    /* Initialization needs to be called once */
    aknano_init(&aknano_settings);
    while (true) {
        static struct aknano_context aknano_context;
        int checkin_result;
        int sleep_time;

        /* Initialize execution context */
        aknano_init_context(&aknano_context, &aknano_settings);

        /*
         * Check-in to device gateway, and select target with the highest version
         * that matches the current tag and hardware type
         */
        checkin_result = aknano_checkin(&aknano_context);
        if (checkin_result == 0) {
            /* Check-in successful. Check selected target */
            bool is_update_required = false;
            bool is_reboot_required = false;

            if (!aknano_has_matching_target(&aknano_context)) {
                LogInfo(("* No matching target found in manifest"));
            } else {
                int current = aknano_get_current(&aknano_context);
                int selected = aknano_get_selected_version(&aknano_context);
                bool is_rollback = aknano_is_rollback(&aknano_context);

                LogInfo(("* Manifest data parsing result: current version=%ld selected version=%ld is_rollback=%s",
                         current, selected, is_rollback? "YES" : "NO"));

                if (is_rollback) {
                    LogInfo(("* Selected version was already applied (and failed). Do not retrying it"));
                } else if (current < selected) {
                    LogInfo((ANSI_COLOR_GREEN "* Update required: %lu -> %ld" ANSI_COLOR_RESET,
                             current, selected));
                    is_update_required = true;
                } else {
                    LogInfo(("* No update required"));
                }
            }

            /* An update is required */
            if (is_update_required)
                is_reboot_required = aknano_install_selected_target(&aknano_context);

            /* An update was performed, reboot board */
            if (is_reboot_required)
                aknano_reboot_command();
        } else {
            LogInfo(("* Check-in failed with error %d", checkin_result));
        }

        sleep_time = aknano_get_setting(&aknano_context, "polling_interval");
        sleep_time = limit_sleep_time_range(sleep_time);
        LogInfo(("Sleeping %d seconds\n\n", sleep_time));
        vTaskDelay(pdMS_TO_TICKS(sleep_time * 1000));
    }
}
