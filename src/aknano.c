/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <time.h>
#include <stdio.h>
#include <string.h>

#include "aknano.h"
#include "aknano_debug.h"
#include "aknano_secret.h"
#include "aknano_flash_storage.h"
#include "aknano_device_gateway.h"
#include "aknano_provisioning.h"

#if defined(AKNANO_ENABLE_EXPLICIT_REGISTRATION) || defined(AKNANO_ALLOW_PROVISIONING) || defined(AKNANO_RESET_DEVICE_ID)
#include "aknano_pkcs11.h"
#endif

// #include "mcuboot_app_support.h"
// #include "task.h"


/**
 * @brief A buffer used in the demo for storing HTTP request headers and
 * HTTP response headers and body.
 *
 * @note This demo shows how the same buffer can be re-used for storing the HTTP
 * response after the HTTP request is sent out. However, the user can also
 * decide to use separate buffers for storing the HTTP request and response.
 */
uint8_t ucUserBuffer[AKNANO_IMAGE_DOWNLOAD_BUFFER_LENGTH];



void aknano_init_settings(struct aknano_settings *aknano_settings)
{
#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    uint32_t temp_value;
#endif

    memset(aknano_settings, 0, sizeof(*aknano_settings));
    strcpy(aknano_settings->tag, "devel");
    aknano_settings->polling_interval = 15;
#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    strcpy(aknano_settings->token, AKNANO_API_TOKEN);
#endif

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    aknano_read_device_certificate(aknano_settings->device_certificate, sizeof(aknano_settings->device_certificate));
#endif
    aknano_settings->image_position = aknano_get_image_position();

    LogInfo(("aknano_init_settings: image_position=%u", aknano_settings->image_position));

    aknano_get_current_version(&aknano_settings->running_version, aknano_settings->image_position);
    if (aknano_settings->running_version == UINT_MAX)
        aknano_settings->running_version = 0;
    LogInfo(("aknano_init_settings: aknano_settings->running_version=%u",
             aknano_settings->running_version));

    aknano_read_flash_storage(AKNANO_FLASH_OFF_DEV_SERIAL, aknano_settings->serial,
                              sizeof(aknano_settings->serial));
    if (aknano_settings->serial[0] < 0)
        aknano_settings->serial[0] = 0;
    LogInfo(("aknano_init_settings: serial=%s", aknano_settings->serial));

    aknano_read_flash_storage(AKNANO_FLASH_OFF_DEV_UUID, aknano_settings->uuid,
                              sizeof(aknano_settings->uuid));
    if (aknano_settings->uuid[0] < 0)
        aknano_settings->uuid[0] = 0;
    LogInfo(("aknano_init_settings: uuid=%s", aknano_settings->uuid));

    aknano_read_flash_storage(AKNANO_FLASH_OFF_LAST_APPLIED_VERSION,
                              &aknano_settings->last_applied_version,
                              sizeof(aknano_settings->last_applied_version));
    if (aknano_settings->last_applied_version > 999999999)
        aknano_settings->last_applied_version = 0;
    LogInfo(("aknano_init_settings: last_applied_version=%d", aknano_settings->last_applied_version));

    aknano_read_flash_storage(AKNANO_FLASH_OFF_LAST_CONFIRMED_VERSION, &aknano_settings->last_confirmed_version,
                              sizeof(aknano_settings->last_confirmed_version));
    if (aknano_settings->last_confirmed_version > 999999999)
        aknano_settings->last_confirmed_version = 0;
    LogInfo(("aknano_init_settings: last_confirmed_version=%d",
             aknano_settings->last_confirmed_version));

    aknano_read_flash_storage(AKNANO_FLASH_OFF_ONGOING_UPDATE_COR_ID,
                              &aknano_settings->ongoing_update_correlation_id,
                              sizeof(aknano_settings->ongoing_update_correlation_id));
    if (aknano_settings->ongoing_update_correlation_id[0] < 0 || aknano_settings->ongoing_update_correlation_id[0] > 122)
        aknano_settings->ongoing_update_correlation_id[0] = 0;
    LogInfo(("aknano_init_settings: ongoing_update_correlation_id=%s",
             aknano_settings->ongoing_update_correlation_id));


    aknano_read_flash_storage(AKNANO_FLASH_OFF_ROLLBACK_RETRY_COUNT,
                              &aknano_settings->rollback_retry_count,
                              sizeof(aknano_settings->rollback_retry_count));
    if (aknano_settings->rollback_retry_count < 0)
        aknano_settings->rollback_retry_count = 0;
    LogInfo(("aknano_init_settings: rollback_retry_count=%d",
             aknano_settings->rollback_retry_count));

    aknano_read_flash_storage(AKNANO_FLASH_OFF_ROLLBACK_NEXT_RETRY_TIME,
                              &aknano_settings->rollback_next_retry_time,
                              sizeof(aknano_settings->rollback_next_retry_time));
    if (aknano_settings->rollback_next_retry_time == UINT32_MAX)
        aknano_settings->rollback_next_retry_time = 0;
    aknano_delay(100);
    LogInfo(("aknano_init_settings: rollback_next_retry_time=%lu",
             aknano_settings->rollback_next_retry_time));

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    ReadFlashStorage(AKNANO_FLASH_OFF_IS_DEVICE_REGISTERED,
                     &temp_value,
                     sizeof(temp_value));
    aknano_settings->is_device_registered = (temp_value & 0xFF) == 1;
    LogInfo(("aknano_init_settings:  is_device_registered=%d",
             aknano_settings->is_device_registered));
#endif

    snprintf(aknano_settings->device_name, sizeof(aknano_settings->device_name),
             "%s-%s",
             aknano_get_board_name(), aknano_settings->serial);

    LogInfo(("aknano_init_settings: device_name=%s",
             aknano_settings->device_name));

    aknano_settings->is_running_rolled_back_image = aknano_settings->last_applied_version
                                                    && aknano_settings->last_applied_version != aknano_settings->running_version
                                                    && strnlen(aknano_settings->ongoing_update_correlation_id, AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH) > 0;

    aknano_delay(100);
    LogInfo(("aknano_init_settings: is_running_rolled_back_image=%d",
             aknano_settings->is_running_rolled_back_image));

    aknano_settings->hwid = aknano_get_board_name();
}

// #define AKNANO_TEST_ROLLBACK
void aknano_send_installation_finished_event(struct aknano_settings *aknano_settings)
{
    static bool executed_once = false;
    bool running_version_reported;

    if (executed_once)
        return;

    executed_once = true;
    running_version_reported = aknano_settings->last_confirmed_version == aknano_settings->running_version;
    LogInfo(("aknano_send_installation_finished_event: aknano_settings.ongoing_update_correlation_id='%s'",
             aknano_settings->ongoing_update_correlation_id));

    if (!aknano_settings->is_running_rolled_back_image && running_version_reported)
        return;

    if (aknano_settings->is_running_rolled_back_image)
        LogInfo(("A rollback was done"));

    aknano_send_event(aknano_settings,
                      AKNANO_EVENT_INSTALLATION_COMPLETED,
                      0, !aknano_settings->is_running_rolled_back_image);
    if (!aknano_settings->is_running_rolled_back_image) {
        aknano_settings->last_applied_version = 0;
        aknano_settings->last_confirmed_version = aknano_settings->running_version;
        aknano_settings->rollback_next_retry_time = 0;
        aknano_settings->rollback_retry_count = 0;
    }
    memset(aknano_settings->ongoing_update_correlation_id, 0,
           sizeof(aknano_settings->ongoing_update_correlation_id));
    aknano_update_settings_in_flash(aknano_settings);
}

#ifdef AKNANO_ALLOW_PROVISIONING
static bool is_certificate_valid(const char *pem)
{
    size_t cert_len;

    if (pem[0] != '-')
        return false;

    cert_len = strnlen(pem, AKNANO_CERT_BUF_SIZE);

    if (cert_len < 200 || cert_len >= AKNANO_CERT_BUF_SIZE)
        return false;

    return true;
}

static bool is_certificate_available_cache = false;
static bool is_valid_certificate_available_()
{
    static bool cert_status;
    char device_certificate[AKNANO_CERT_BUF_SIZE];

    cert_status = aknano_read_device_certificate(device_certificate, sizeof(device_certificate));
    if (!cert_status)
        is_certificate_available_cache = false;
    else
        is_certificate_available_cache = is_certificate_valid(device_certificate);
    LogInfo(("Device certificate available? %s", is_certificate_available_cache? "YES": "NO"));
    return is_certificate_available_cache;
}

bool is_valid_certificate_available(bool use_cached_value)
{
    if (use_cached_value || is_certificate_available_cache)
        return is_certificate_available_cache;

    return is_valid_certificate_available_();
}

bool is_device_serial_set()
{
    char serial[AKNANO_MAX_SERIAL_LENGTH];
    bool is_serial_set;

    aknano_read_flash_storage(AKNANO_FLASH_OFF_DEV_SERIAL, serial, sizeof(serial));
    if (serial[0] == 0xff)
        serial[0] = 0;
    LogInfo(("aknano_init_settings: serial=%s", serial));
    is_serial_set = strnlen(serial, sizeof(serial)) > 5;
    LogInfo(("Device serial set? %s", is_serial_set? "YES": "NO"));
    return is_serial_set;
}
#endif

#if defined(AKNANO_ENABLE_EL2GO) && defined(AKNANO_ALLOW_PROVISIONING)
extern bool el2go_agent_stopped;
#endif

void aknano_log_running_mode()
{
    LogInfo((ANSI_COLOR_YELLOW "start_aknano mode '" AKNANO_PROVISIONING_MODE "'" ANSI_COLOR_RESET));
#ifdef AKNANO_RESET_DEVICE_ID
    LogInfo((ANSI_COLOR_YELLOW "Reset of device provisioned data is enabled" ANSI_COLOR_RESET));
#endif
#ifdef AKNANO_ALLOW_PROVISIONING
    LogInfo((ANSI_COLOR_YELLOW "Provisioning support is enabled" ANSI_COLOR_RESET));
#endif
}

void aknano_init(struct aknano_settings *aknano_settings)
{
#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    bool registrationOk;
#endif
    LogInfo((AKNANO_TEST_MESSAGE_PREAMBLE " aknano_init AKNANO_HASH=" AKNANO_COMMIT_ID));
    LogInfo((AKNANO_TEST_MESSAGE_PREAMBLE " aknano_init MANIFEST_HASH=" AKNANO_MANIFEST_COMMIT_ID));

#ifdef AKNANO_TEST
    LogInfo(("aknano_run_tests Begin"));
    aknano_run_tests();
    LogInfo(("aknano_run_tests Done"));
    aknano_delay(1000);
#endif

    aknano_log_running_mode();
    LogInfo(("Initializing ak-nano..."));

#ifdef AKNANO_RESET_DEVICE_ID
    LogWarn((ANSI_COLOR_RED "AKNANO_RESET_DEVICE_ID is set. Removing provisioned device data" ANSI_COLOR_RESET));
    aknano_clear_provisioned_data();
    prvDestroyDefaultCryptoObjects();
#ifdef AKNANO_ENABLE_EL2GO
    LogWarn((ANSI_COLOR_RED "Halting execution" ANSI_COLOR_RESET));
    for (;;);
#endif
#endif

#ifdef AKNANO_ALLOW_PROVISIONING
#ifdef AKNANO_ENABLE_EL2GO
    if (!is_device_serial_set()) {
        LogWarn((ANSI_COLOR_RED "Device Serial is not set. Running initial provisioning process" ANSI_COLOR_RESET));
        aknano_provision_device();
        aknano_delay(1000);
        if (!is_device_serial_set()) {
            LogError((ANSI_COLOR_RED "Fatal: Error fetching device serial" ANSI_COLOR_RESET));
            aknano_delay(120000);
        }
    } else {
        LogInfo(("Device serial is set"));
    }
#else
    if (!is_device_serial_set() || !is_valid_certificate_available(false)) {
        LogWarn((ANSI_COLOR_RED "Device certificate (and/or serial) is not set. Running provisioning process" ANSI_COLOR_RESET));
        aknano_provision_device();
        aknano_delay(1000);
        if (!is_valid_certificate_available(false)) {
            LogError((ANSI_COLOR_RED "Fatal: Error fetching device certificate" ANSI_COLOR_RESET));
            aknano_delay(120000);
        }
    } else {
        LogInfo(("Device certificate and serial are set"));
    }
#endif
#endif

#if defined(AKNANO_ENABLE_EL2GO) && defined(AKNANO_ALLOW_PROVISIONING)
    LogInfo(("EL2Go provisioning enabled. Waiting for secure objects to be retrieved"));
    while (!is_valid_certificate_available(true) || !el2go_agent_stopped)
        aknano_delay(1000);
    LogInfo(("EL2GO provisioning succeeded. Proceeding"));
#endif
    LogInfo(("Initializing settings..."));
    aknano_init_settings(aknano_settings);

    aknano_delay(100);
    LogInfo((AKNANO_TEST_MESSAGE_PREAMBLE "aknano_init UUID=%s", aknano_settings->uuid));
    LogInfo((AKNANO_TEST_MESSAGE_PREAMBLE "aknano_init RUNNING_VERSION=%u", aknano_settings->running_version));
    LogInfo((AKNANO_TEST_MESSAGE_PREAMBLE "aknano_init RUNNING_FROM_SLOT=%u", aknano_settings->image_position));

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    if (!xaknano_settings.is_device_registered) {
        registrationOk = aknano_register_device(&xaknano_settings);
        if (registrationOk) {
            xaknano_settings.is_device_registered = registrationOk;
            aknano_update_settings_in_flash(&xaknano_settings);
        }
    }
#endif

#ifdef AKNANO_DELETE_PROVISIONED_TUF_ROOT
    LogWarn((ANSI_COLOR_RED "**** Reseting factory TUF 1.root.json ****" ANSI_COLOR_RESET));
    aknano_clear_provisioned_tuf_root();
#endif

#ifdef AKNANO_DELETE_TUF_DATA
    LogWarn((ANSI_COLOR_RED "**** Reseting TUF data ****" ANSI_COLOR_RESET));
#include "libtufnano.h"
    tuf_client_write_local_file(ROLE_ROOT, "\xFF", 1, NULL);
    tuf_client_write_local_file(ROLE_TIMESTAMP, "\xFF", 1, NULL);
    tuf_client_write_local_file(ROLE_SNAPSHOT, "\xFF", 1, NULL);
    tuf_client_write_local_file(ROLE_TARGETS, "\xFF", 1, NULL);

    LogWarn((ANSI_COLOR_RED "**** Sleeping for 20 seconds ****" ANSI_COLOR_RESET));
    aknano_delay(20000);
#endif
    aknano_settings->is_image_permanent = aknano_is_current_image_permanent();
}
