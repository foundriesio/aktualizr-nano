/*
 * Copyright 2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <time.h>
#include <stdio.h>
#include <string.h>

#include "backoff_algorithm.h"
#include "core_json.h"
#include "ini.h"

#include "aknano.h"
#include "aknano_debug.h"
#include "aknano_client.h"
#include "aknano_net.h"
#include "aknano_secret.h"
#include "aknano_flash_storage.h"
#include "aknano_image_download.h"
#include "aknano_device_gateway.h"
#include "aknano_board.h"
#include "aknano_public_api.h"
#include "aknano_tuf_client.h"
#include "aknano_targets_manifest.h"

// #include "flexspi_flash_config.h"
#include "libtufnano.h"

#define AKNANO_MAX_ROLLED_BACK_VERSION_RETRIES  5

/* 5 minutes */
#define AKNANO_ROLLBACK_RETRY_BACKOFF_BASE 5

/* 10 days */
#define AKNANO_ROLLBACK_RETRY_MAX_DELAY 60 * 24 * 10

/*
 * TODO: UpdateSettingValue is defined at client application, but we could have a better interface for that
 *
 */
void UpdateSettingValue(const char *, int);


static void fill_network_info(char *output, size_t max_length)
{
    uint8_t ipv4[4];
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

static int toml_handler(void *user, const char *section, const char *name,
                        const char *value)
{
    struct aknano_settings *aknano_settings = (struct aknano_settings *)user;

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
    if (MATCH("pacman", "tags")) {
        strncpy(aknano_settings->tag, value + 1, sizeof(aknano_settings->tag)-1);
        aknano_settings->tag[strlen(aknano_settings->tag) - 1] = 0; /* strip last " */
    }
    return 1;
}

static void replace_escaped_chars(char *dst, const char *src, size_t len)
{
    const char *p;
    char *d;

    for (p = src, d = dst; p < (src + len); p++) {
        if (*p == '\\' && p < (src + len - 1)) {
            switch (*(p + 1)) {
            case 'n': *(d++) = '\n'; break;
            case 'r': *(d++) = '\r'; break;
            case '"': *(d++) = '"'; break;
            case '\\': *(d++) = '\\'; break;
            }
            p++;
        } else {
            *(d++) = *p;
        }
    }
    *d = 0;
}

static void parse_config(const char *config_data, int buffer_len, struct aknano_settings *aknano_settings)
{
    JSONStatus_t result = JSON_Validate(config_data, buffer_len);
    const char *value;
    size_t value_length;
    int int_value;
    JSONTypes_t data_type;

    if (result == JSONSuccess) {
        static char unescaped_toml[200];

        memset(unescaped_toml, 0, sizeof(unescaped_toml));
        result = JSON_SearchConst(config_data, buffer_len,
                             "z-50-fioctl.toml" TUF_JSON_QUERY_KEY_SEPARATOR "Value",
                             strlen("z-50-fioctl.toml" TUF_JSON_QUERY_KEY_SEPARATOR "Value"),
                             &value, &value_length, &data_type);
        if (value_length < sizeof(unescaped_toml)) {
            replace_escaped_chars(unescaped_toml, value, value_length);
            ini_parse_string(unescaped_toml, toml_handler, aknano_settings);
        } else {
            LogWarn(("z-50-fioctl.toml too big, skipping parsing. Size=%ld, limit=%ld", value_length, sizeof(unescaped_toml)));
        }

        result = JSON_SearchConst(config_data, buffer_len,
                             "polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value",
                             strlen("polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value"),
                             &value, &value_length, &data_type);

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

        result = JSON_SearchConst(config_data, buffer_len,
                             "btn_polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value",
                             strlen("btn_polling_interval" TUF_JSON_QUERY_KEY_SEPARATOR "Value"),
                             &value, &value_length, &data_type);

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
    static char body_buffer[250];

    struct aknano_network_context network_context;
    BaseType_t xDemoStatus;
    struct aknano_settings *aknano_settings = aknano_context->settings;
    int tuf_ret = TUF_SUCCESS;

#ifdef AKNANO_ALLOW_PROVISIONING
    static bool tuf_root_is_provisioned = false;
#endif
#ifdef AKNANO_DUMP_MEMORY_USAGE_INFO
    aknano_dump_memory_info("Before aknano_checkin");
#endif
    LogInfo(("aknano_checkin. Version=%u  Tag=%s", aknano_settings->running_version, aknano_settings->tag));

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
                // network_context.reply_body[network_context.reply_body_len] = 0;
                parse_config((const char *)network_context.reply_body, network_context.reply_body_len, aknano_context->settings);
        }

        snprintf(body_buffer, sizeof(body_buffer),
                 "{ " \
                 " \"product\": \"%s\"," \
                 " \"description\": \"Aktualizr-nano PoC\"," \
                 " \"claimed\": true, " \
                 " \"serial\": \"%s\", " \
                 " \"id\": \"%s\", " \
                 " \"class\": \"MCU\" " \
                 "}",
                 aknano_get_board_name(), aknano_settings->serial, aknano_get_board_name());

        aknano_send_http_request(&network_context, HTTP_METHOD_PUT,
                                 "/system_info", body_buffer, strlen(body_buffer),
                                 aknano_context->settings);

        fill_network_info(body_buffer, sizeof(body_buffer));
        aknano_send_http_request(&network_context, HTTP_METHOD_PUT,
                                 "/system_info/network", body_buffer, strlen(body_buffer),
                                 aknano_context->settings);

        // LogInfo(("aknano_settings->tag=%s",aknano_settings->tag));
        sprintf(body_buffer,
                "[aknano_settings]\n" \
                "poll_interval = %d\n" \
                "hw_id = \"%s\"\n" \
                "tag = \"%s\"\n" \
                "binary_compilation_local_time = \""__DATE__ " " __TIME__ "\"\n"
                "provisioning_mode = \"" AKNANO_PROVISIONING_MODE "\"",
                aknano_settings->polling_interval,
                aknano_get_board_name(),
                aknano_settings->tag);
        aknano_send_http_request(&network_context, HTTP_METHOD_PUT,
                                 "/system_info/config", body_buffer, strlen(body_buffer),
                                 aknano_context->settings);

        time_t reference_time = aknano_cli_get_current_epoch();
// #define TUF_FORCE_DATE_IN_FUTURE 1
#ifdef TUF_FORCE_DATE_IN_FUTURE
        LogInfo((ANSI_COLOR_RED "Forcing TUF reference date to be 1 year from now" ANSI_COLOR_RESET));
        reference_time += 31536000; // Add 1 year
#endif
        aknano_context->dg_network_context = &network_context;

#ifdef AKNANO_TEST_ROLLBACK
#warning "Compiling broken image for rollback test"
        LogError((ANSI_COLOR_RED "This is a rollback test. Rebooting in 5 seconds" ANSI_COLOR_RESET));
        aknano_delay(5000);
        NVIC_SystemReset();
#endif

#ifdef AKNANO_ALLOW_PROVISIONING
        if (!tuf_root_is_provisioned) {
            tuf_ret = aknano_provision_tuf_root(aknano_context);
            tuf_root_is_provisioned = (tuf_ret == 0);
        }
#endif
        if (tuf_ret == TUF_SUCCESS) {
            /* Leave some room for http headers inside the same buffer */
            const size_t max_tuf_metadata_size = sizeof(ucUserBuffer) - 1024;
            tuf_ret = tuf_refresh(aknano_context, reference_time, ucUserBuffer, max_tuf_metadata_size);
            LogInfo((ANSI_COLOR_MAGENTA "tuf_refresh %s (%d)" ANSI_COLOR_RESET, tuf_get_error_string(tuf_ret), tuf_ret));
        }

        if (tuf_ret == TUF_SUCCESS) {
            /* ucUserBuffer is shared between TUF and overall HTTP communication
             * Make sure to handle returned TUF data before performing any new HTTP operation
             */
            parse_targets_metadata((const char *)ucUserBuffer, strlen((char *)ucUserBuffer), aknano_context);

            /* Wait for a successful tuf refresh before marking image as permanent */
            if (aknano_settings->application_self_test_ok) {
                aknano_set_image_confirmed(aknano_settings);
                aknano_send_installation_finished_event(aknano_settings);
            }
        }
    } else {
        tuf_ret = -10;
    }
    /* Close the network connection.  */
    aknano_mtls_disconnect(&network_context);
#ifdef AKNANO_DUMP_MEMORY_USAGE_INFO
    aknano_dump_memory_info("After aknano_checkin");
#endif
    return tuf_ret;
}

bool aknano_is_rollback(struct aknano_context *aknano_context)
{
    return aknano_context->selected_target.version == aknano_context->settings->last_applied_version;
}

time_t aknano_get_next_rollback_retry_time(struct aknano_settings *aknano_settings)
{
    // BackoffAlgorithmStatus_t retry_status = BackoffAlgorithmSuccess;
    BackoffAlgorithmContext_t retry_params;
    uint16_t next_retry_backoff = 0; /* in minutes */
    uint32_t random_int;

    BackoffAlgorithm_InitializeParams(&retry_params,
                                      AKNANO_ROLLBACK_RETRY_BACKOFF_BASE,
                                      AKNANO_ROLLBACK_RETRY_MAX_DELAY,
                                      AKNANO_MAX_ROLLED_BACK_VERSION_RETRIES);

    aknano_cli_gen_random_bytes((char *)&random_int, sizeof(random_int));
    for (int i = 0; i < aknano_settings->rollback_retry_count; i++)
        BackoffAlgorithm_GetNextBackoff(&retry_params, random_int, &next_retry_backoff);

    LogInfo(("rollback_retry set to %u minutes", next_retry_backoff));
    return aknano_cli_get_current_epoch() + (next_retry_backoff * 60);
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
                      true);
    if (aknano_download_and_flash_image(aknano_context)) {
        aknano_send_event(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_COMPLETED,
                          aknano_context->selected_target.version,
                          true);
        aknano_send_event(aknano_context->settings, AKNANO_EVENT_INSTALLATION_STARTED,
                          aknano_context->selected_target.version,
                          true);

        aknano_settings->last_applied_version = aknano_context->selected_target.version;
        aknano_send_event(aknano_context->settings, AKNANO_EVENT_INSTALLATION_APPLIED,
                          aknano_context->selected_target.version,
                          true);

        LogInfo(("Requesting update on next boot (ReadyForTest)"));
        status_t status;
        status = aknano_set_image_ready_for_test();
        if (status)
            LogWarn(("Error setting image as ReadyForTest. status=%d", status));
        else
            is_reboot_required = true;
    } else {
        aknano_send_event(aknano_context->settings, AKNANO_EVENT_DOWNLOAD_COMPLETED,
                          aknano_context->selected_target.version,
                          false);
    }

    if (is_reboot_required) {
        // if (aknano_settings->is_running_rolled_back_image && aknano_is_rollback(aknano_context)) {
        aknano_settings->rollback_retry_count++;
        // }
        aknano_settings->rollback_next_retry_time = aknano_get_next_rollback_retry_time(aknano_settings);
        LogInfo(("*** rollback_retry_count=%u rollback_next_retry_time=%lu",
                 aknano_settings->rollback_retry_count, aknano_settings->rollback_next_retry_time));
    }

    aknano_update_settings_in_flash(aknano_settings);
    return is_reboot_required;
}

bool aknano_set_application_self_test_ok(struct aknano_settings *aknano_settings)
{
    return aknano_settings->application_self_test_ok = true;
}

bool aknano_is_temp_image(struct aknano_settings *aknano_settings)
{
    return !aknano_settings->is_image_permanent;
}

bool aknano_has_matching_target(struct aknano_context *aknano_context)
{
    return aknano_context->selected_target.version != 0;
}

bool aknano_should_retry_rollback(struct aknano_context *aknano_context)
{
    if (aknano_context->settings->rollback_retry_count >= AKNANO_MAX_ROLLED_BACK_VERSION_RETRIES)
        return false;

    if (aknano_context->settings->rollback_next_retry_time > aknano_cli_get_current_epoch())
        return false;

    return true;
}

uint32_t aknano_get_current(struct aknano_context *aknano_context)
{
    return aknano_context->settings->running_version;
}

uint32_t aknano_get_selected_version(struct aknano_context *aknano_context)
{
    return aknano_context->selected_target.version;
}

int aknano_get_setting(struct aknano_context *aknano_context, const char *setting_name)
{
    if (!strncmp(setting_name, "polling_interval", strlen("polling_interval")))
        return aknano_context->settings->polling_interval;
    LogError(("Invalid setting name %s", setting_name));
    return 0;
}

int aknano_limit_sleep_time_range(int sleep_time)
{
    /* No less than 5 seconds, no more than 1 hour */
    if (sleep_time < 5)
        return 5;
    else if (sleep_time > 60 * 60)
        return 60 * 60;
    else
        return sleep_time;
}

void aknano_sample_loop(uint32_t *remaining_iterations)
{
    static struct aknano_settings aknano_settings;
    time_t startup_epoch = aknano_cli_get_current_epoch();
    const time_t max_offline_time_on_temp_image = 180;
    bool any_checkin_ok = false;

    /* Initialization needs to be called once */
    aknano_init(&aknano_settings);

    /*
     * Tell aktualizr-nano that it is OK to set the current image as permanent
     * aktualizr-nano also will only set the image as permanent after a successful
     * checkin is done at the factory device gateway
     */
    aknano_set_application_self_test_ok(&aknano_settings);
    while (remaining_iterations == NULL || *remaining_iterations > 0) {
        static struct aknano_context aknano_context;
        int checkin_result;
        int sleep_time;

        if (remaining_iterations != NULL)
            (*remaining_iterations)--;

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
            if (!any_checkin_ok) {
                LogInfo((AKNANO_TEST_MESSAGE_PREAMBLE "Checkin successful"));
            }

            any_checkin_ok = true;
            if (aknano_has_matching_target(&aknano_context)) {
                uint32_t current = aknano_get_current(&aknano_context);
                uint32_t selected = aknano_get_selected_version(&aknano_context);
                bool is_rollback = aknano_is_rollback(&aknano_context);
                bool should_retry_rollback = false;

                if (is_rollback)
                    should_retry_rollback = aknano_should_retry_rollback(&aknano_context);

                LogInfo(("* Manifest data parsing result: current version=%u selected version=%u is_rollback=%s should_retry_rollback=%s",
                         current, selected, is_rollback? "YES" : "NO", should_retry_rollback? "YES" : "NO"));

                if (is_rollback && !should_retry_rollback) {
                    LogInfo(("* Selected version was already applied (and failed). Do not retrying it"));
                } else if (current < selected) {
                    LogInfo((ANSI_COLOR_GREEN "* Update required: %u -> %u" ANSI_COLOR_RESET,
                             current, selected));
                    is_update_required = true;
                } else {
                    LogInfo(("* No update required"));
                }
            } else {
                LogInfo(("* No matching target found in manifest"));
            }

            /* An update is required */
            if (is_update_required)
                is_reboot_required = aknano_install_selected_target(&aknano_context);

            /* An update was performed, reboot board */
            if (is_reboot_required) {
                aknano_reboot_command();
                return;
            }
        } else {
            LogInfo(("* Check-in failed with error %d", checkin_result));
        }

        /* If the checkin operation fails for too long after an update, the image may be bad */
        if (!any_checkin_ok && aknano_is_temp_image(&aknano_settings) &&
            aknano_cli_get_current_epoch() > startup_epoch + max_offline_time_on_temp_image) {
            LogWarn(("* Check-in failed for too long while running a temporary image. Forcing a reboot to initiate rollback process"));
            aknano_delay(2000);
            aknano_reboot_command();
            return;
        }

        sleep_time = aknano_get_setting(&aknano_context, "polling_interval");
        sleep_time = aknano_limit_sleep_time_range(sleep_time);
        LogInfo(("Sleeping %d seconds. any_checkin_ok=%d temp_image=%d\n\n",
                 sleep_time, any_checkin_ok, aknano_is_temp_image(&aknano_settings)));

        if (remaining_iterations == NULL || *remaining_iterations > 0)
            aknano_delay(sleep_time * 1000);
    }
}
