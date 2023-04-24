/*
 * Copyright 2021-2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_PRIV_H__
#define __AKNANO_PRIV_H__

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "mbedtls/sha256.h"

#include "aknano_board.h"

#define RECV_BUFFER_SIZE 1640
#define URL_BUFFER_SIZE 300

#define AKNANO_MAX_TAG_LENGTH 32
#define AKNANO_MAX_UPDATE_AT_LENGTH 32
#define AKNANO_MAX_URI_LENGTH 120
#define AKNANO_CERT_BUF_SIZE 1024
#define AKNANO_MAX_DEVICE_NAME_SIZE 100
#define AKNANO_MAX_UUID_LENGTH 40
#define AKNANO_MAX_SERIAL_LENGTH 50
#define AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH 100

#define AKNANO_MAX_FIRMWARE_SIZE 0x100000

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
#define AKNANO_MAX_TOKEN_LENGTH 100
#endif

#ifndef AKNANO_DEFAULT_TAG
#define AKNANO_DEFAULT_TAG "devel"
#endif
/*
 * Aktualizr-nano storage
 */

/* Flash sector size. Usually, a whole sector needs to be erased at once */
#define AKNANO_FLASH_SECTOR_SIZE 4096

/* Physical address on the storage device where aknano data will be stored */
#define AKNANO_STORAGE_FLASH_OFFSET 0x600000

/* Number of flash sectors used by aknano, including TUF metadata */
#define AKNANO_FLASH_SECTORS_COUNT 22

/* Offset of each data field, relative to AKNANO_STORAGE_FLASH_OFFSET */

/* Basic device ID */
#define AKNANO_FLASH_OFF_DEV_ID_BASE 0
#define AKNANO_FLASH_OFF_DEV_UUID   (AKNANO_FLASH_OFF_DEV_ID_BASE + 0)
#define AKNANO_FLASH_OFF_DEV_SERIAL (AKNANO_FLASH_OFF_DEV_ID_BASE + 128)

/* aknano state */
#define AKNANO_FLASH_OFF_STATE_BASE (AKNANO_FLASH_SECTOR_SIZE * 1)
#define AKNANO_FLASH_OFF_LAST_APPLIED_VERSION   (AKNANO_FLASH_OFF_STATE_BASE + 0)
#define AKNANO_FLASH_OFF_LAST_CONFIRMED_VERSION (AKNANO_FLASH_OFF_STATE_BASE + sizeof(int))
#define AKNANO_FLASH_OFF_ONGOING_UPDATE_COR_ID  (AKNANO_FLASH_OFF_STATE_BASE + sizeof(int) * 2)

#define AKNANO_FLASH_OFF_ROLLBACK_RETRY_COUNT  (AKNANO_FLASH_OFF_STATE_BASE + (sizeof(int) * 2) + 100)
#define AKNANO_FLASH_OFF_ROLLBACK_NEXT_RETRY_TIME  (AKNANO_FLASH_OFF_STATE_BASE + (sizeof(int) * 3) + 100)

/* TUF metadata */
#define AKNANO_FLASH_OFF_TUF_ROLES_BASE (AKNANO_FLASH_SECTOR_SIZE * 2)
#define AKNANO_TUF_METADATA_MAX_SIZE (AKNANO_FLASH_SECTOR_SIZE * 4) /* 16 KB */
#define AKNANO_FLASH_OFF_TUF_ROOT_PROVISIONING (AKNANO_FLASH_OFF_TUF_ROLES_BASE + (AKNANO_TUF_METADATA_MAX_SIZE * 0))
#define AKNANO_FLASH_OFF_TUF_ROLE_ROOT         (AKNANO_FLASH_OFF_TUF_ROLES_BASE + (AKNANO_TUF_METADATA_MAX_SIZE * 1))
#define AKNANO_FLASH_OFF_TUF_ROLE_TIMESTAMP    (AKNANO_FLASH_OFF_TUF_ROLES_BASE + (AKNANO_TUF_METADATA_MAX_SIZE * 2))
#define AKNANO_FLASH_OFF_TUF_ROLE_SNAPSHOT     (AKNANO_FLASH_OFF_TUF_ROLES_BASE + (AKNANO_TUF_METADATA_MAX_SIZE * 3))
#define AKNANO_FLASH_OFF_TUF_ROLE_TARGETS      (AKNANO_FLASH_OFF_TUF_ROLES_BASE + (AKNANO_TUF_METADATA_MAX_SIZE * 4))

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
#define AKNANO_FLASH_OFF_IS_DEVICE_REGISTERED AKNANO_FLASH_OFF_STATE_BASE + sizeof(int) * 2 + AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH
#endif

/*
 * Altualizr-nano event fields
 */
#define AKNANO_EVENT_DOWNLOAD_STARTED "EcuDownloadStarted"
#define AKNANO_EVENT_DOWNLOAD_COMPLETED "EcuDownloadCompleted"
#define AKNANO_EVENT_INSTALLATION_STARTED "EcuInstallationStarted"
#define AKNANO_EVENT_INSTALLATION_APPLIED "EcuInstallationApplied"
#define AKNANO_EVENT_INSTALLATION_COMPLETED "EcuInstallationCompleted"

#define AKNANO_EVENT_SUCCESS_UNDEFINED 0
#define AKNANO_EVENT_SUCCESS_FALSE 1
#define AKNANO_EVENT_SUCCESS_TRUE 2

#define AKNANO_IMAGE_DOWNLOAD_REQUEST_LENGTH  4096 * 4
#define AKNANO_IMAGE_DOWNLOAD_BUFFER_LENGTH AKNANO_IMAGE_DOWNLOAD_REQUEST_LENGTH + 1024

#define AKNANO_SHA256_LEN 32

#ifdef AKNANO_ENABLE_EL2GO
#define AKNANO_PROVISIONING_MODE "EdgeLock 2GO Managed"
#else
#ifdef AKNANO_ENABLE_SE05X
#define AKNANO_PROVISIONING_MODE "SE05X Standalone"
#else
#define AKNANO_PROVISIONING_MODE "No Secure Element"
#endif
#endif

#ifndef AKNANO_TEST_MESSAGE_PREAMBLE
#define AKNANO_TEST_MESSAGE_PREAMBLE ""
#endif

#ifndef AKNANO_COMMIT_ID
#define AKNANO_COMMIT_ID "UNKNOWN"
#endif

#ifndef AKNANO_MANIFEST_COMMIT_ID
#define AKNANO_MANIFEST_COMMIT_ID "UNKNOWN"
#endif

/*
 * Altualizr-nano internal structs
 */
struct aknano_target {
    char    updatedAt[AKNANO_MAX_UPDATE_AT_LENGTH];
    size_t  expected_size;
    uint32_t version;
    uint8_t expected_hash[AKNANO_SHA256_LEN];
};

/* Settings are kept between iterations */
struct aknano_settings {
    char        tag[AKNANO_MAX_TAG_LENGTH];
    char        device_name[AKNANO_MAX_DEVICE_NAME_SIZE];
    char        uuid[AKNANO_MAX_UUID_LENGTH];
    char        serial[AKNANO_MAX_SERIAL_LENGTH];
    uint32_t    running_version;
    uint32_t    last_applied_version;
    uint32_t    last_confirmed_version;
    int         rollback_retry_count;
    time_t      rollback_next_retry_time;
    int         polling_interval;
    time_t      boot_up_epoch;
    char        ongoing_update_correlation_id[AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH];
    uint8_t     image_position;
    const char *hwid;
    bool        is_running_rolled_back_image;
    bool        application_self_test_ok;
    bool        is_image_permanent;
#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    char        token[AKNANO_MAX_TOKEN_LENGTH];
    char        device_certificate[AKNANO_CERT_BUF_SIZE];
    bool        is_device_registered;
#endif
};


/* Context is not kept between iterations */
struct aknano_network_context;
struct aknano_context {
    uint8_t                        url_buffer[URL_BUFFER_SIZE];
    struct aknano_settings *       settings;
    struct aknano_target           selected_target;

    /* Connection to the device gateway */
    struct aknano_network_context *dg_network_context;

    mbedtls_sha256_context         sha256_context;
};


extern uint8_t ucUserBuffer[AKNANO_IMAGE_DOWNLOAD_BUFFER_LENGTH];
bool is_valid_certificate_available(bool);

void aknano_delay(uint32_t ms);

#endif /* __AKNANO_PRIV_H__ */
