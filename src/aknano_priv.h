/*
 * Copyright 2021-2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_PRIV_H__
#define __AKNANO_PRIV_H__

#include "mbedtls/sha256.h"
#include "core_pkcs11_config.h"
#include "core_pkcs11.h"
#include "pkcs11t.h"
#include "board.h"

#include "fsl_common.h"
#include <time.h>

#include "mcuboot_app_support.h"
/* Transport interface implementation include header for TLS. */
#include "transport_secure_sockets.h"

#include "fsl_flexspi.h"
#include "aws_demo_config.h"
#include "aws_dev_mode_key_provisioning.h"

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "core_json.h"

#include "iot_network.h"

/* Include PKCS11 helper for random number generation. */
#include "pkcs11_helpers.h"

/*Include backoff algorithm header for retry logic.*/
// #include "backoff_algorithm.h"

/* Transport interface include. */
#include "transport_interface.h"

/* Transport interface implementation include header for TLS. */
#include "transport_secure_sockets.h"

/* Include header for connection configurations. */
#include "aws_clientcredential.h"

#include "core_http_client.h"

// #define AKNANO_DRY_RUN

#define AKNANO_BOARD_NAME BOARD_NAME
#define TUF_DATA_BUFFER_LEN 10 * 1024

#define RECV_BUFFER_SIZE 1640
#define URL_BUFFER_SIZE 300

#define AKNANO_MAX_TAG_LENGTH 32
#define AKNANO_MAX_UPDATE_AT_LENGTH 32
#define AKNANO_MAX_URI_LENGTH 120
#define AKNANO_CERT_BUF_SIZE 1024
#define AKNANO_MAX_DEVICE_NAME_SIZE 100
#define AKNANO_MAX_UUID_LENGTH 100
#define AKNANO_MAX_SERIAL_LENGTH 100
#define AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH 100

#define AKNANO_MAX_FIRMWARE_SIZE 0x100000

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
#define AKNANO_MAX_TOKEN_LENGTH 100
#endif

/*
 * Aktualizr nano storage
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

/*
 * Generic ANSI colors escape codes, used in console output
 */
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"


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

extern unsigned char tuf_data_buffer[TUF_DATA_BUFFER_LEN];

enum aknano_response {
    AKNANO_NETWORKING_ERROR,
    AKNANO_UNCONFIRMED_IMAGE,
    AKNANO_METADATA_ERROR,
    AKNANO_DOWNLOAD_ERROR,
    AKNANO_OK,
    AKNANO_UPDATE_INSTALLED,
    AKNANO_NO_UPDATE,
    AKNANO_CANCEL_UPDATE,
};

struct aknano_target {
    char    updatedAt[AKNANO_MAX_UPDATE_AT_LENGTH];
    size_t  expected_size;
    int32_t version;
    uint8_t expected_hash[AKNANO_SHA256_LEN];
};

/* Settings are kept between iterations */
struct aknano_settings {
    char        tag[AKNANO_MAX_TAG_LENGTH];
    char        device_name[AKNANO_MAX_DEVICE_NAME_SIZE];
    char        uuid[AKNANO_MAX_UUID_LENGTH];
    char        serial[AKNANO_MAX_SERIAL_LENGTH];
    uint32_t    running_version;
    int         last_applied_version;
    int         last_confirmed_version;
    int         polling_interval;
    time_t      boot_up_epoch;
    char        ongoing_update_correlation_id[AKNANO_MAX_UPDATE_CORRELATION_ID_LENGTH];
    uint8_t     image_position;
    const char *hwid;
#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    char        token[AKNANO_MAX_TOKEN_LENGTH];
    char        device_certificate[AKNANO_CERT_BUF_SIZE];
    bool        is_device_registered;
#endif
};

struct aknano_network_context;

/* Context is not kept between iterations */
struct aknano_context {
    uint8_t                        url_buffer[URL_BUFFER_SIZE];
    struct aknano_settings *       settings; /* TODO: may not always be set yet */
    struct aknano_target           selected_target;

    /* Connection to the device gateway */
    struct aknano_network_context *dg_network_context;

    mbedtls_sha256_context         sha256_context;
};


#define AKNANO_IMAGE_DOWNLOAD_REQUEST_LENGTH  4096 * 4
#define AKNANO_IMAGE_DOWNLOAD_BUFFER_LENGTH AKNANO_IMAGE_DOWNLOAD_REQUEST_LENGTH + 1024

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


/**/
void aknano_update_settings_in_flash(struct aknano_settings *aknano_settings);

status_t aknano_init_flash_storage();
status_t aknano_read_flash_storage(int offset, void *output, size_t outputMaxLen);
status_t aknano_write_data_to_flash(int offset, const void *data, size_t data_len);
status_t aknano_write_data_to_storage(int offset, const void *data, size_t data_len);

int init_network_context(struct aknano_network_context *network_context);

BaseType_t aknano_mtls_connect(struct aknano_network_context *network_context, const char *hostname, size_t hostname_len, uint16_t port, const char *server_root_ca, size_t server_root_ca_len);

BaseType_t aknano_mtls_send_http_request(struct aknano_network_context *network_context, const char *hostname, size_t hostname_len, const char *pcMethod, const char *pcPath, const char *pcBody, size_t xBodyLen, unsigned char *buffer, size_t buffer_len, const char **header_keys, const char **header_values, size_t header_len);

void aknano_mtls_disconnect(struct aknano_network_context *network_context);

int aknano_download_and_flash_image(struct aknano_context *aknano_context);
void aknano_update_settings_in_flash(struct aknano_settings *aknano_settings);
bool aknano_send_event(struct aknano_settings *aknano_settings, const char *event_type, int new_version, bool success);

/* API */
void aknano_init_settings(struct aknano_settings *aknano_settings);
void aknano_init_context(struct aknano_context *aknano_context, struct aknano_settings *aknano_settings);
void aknano_send_installation_finished_event(struct aknano_settings *aknano_settings);
void aknano_set_image_confirmed();
void aknano_init(struct aknano_settings *aknano_settings);

int parse_targets_metadata(const char *data, int len, void *application_context);

status_t aknano_save_uuid_and_serial(const char *uuid, const char *serial);

extern uint8_t ucUserBuffer[AKNANO_IMAGE_DOWNLOAD_BUFFER_LENGTH];
BaseType_t aknano_connect_to_device_gateway(struct aknano_network_context *network_context);

BaseType_t aknano_send_http_request(struct aknano_network_context *network_context, const char *pcMethod, const char *pcPath, const char *pcBody, size_t xBodyLen, struct aknano_settings *aknano_settings);

int aknano_gen_serial_and_uuid(char *uuid_string, char *serial_string);

void aknano_handle_manifest_data(struct aknano_context *context, uint8_t *dst, off_t *offset, uint8_t *src, size_t len);

void aknano_get_ipv4_and_mac(char *ipv4, uint8_t *mac);

void aknano_dump_memory_info(const char *context);

void aknano_sample_loop();

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
bool aknano_register_device(struct aknano_settings *aknano_settings);
#endif

#if defined(AKNANO_ENABLE_EXPLICIT_REGISTRATION) || defined(AKNANO_ALLOW_PROVISIONING)
CK_RV aknano_read_device_certificate(char *dst, size_t dst_size);
#endif

#ifdef AKNANO_RESET_DEVICE_ID
CK_RV prvDestroyDefaultCryptoObjects(void);
#endif

#ifdef AKNANO_ALLOW_PROVISIONING
void vDevModeKeyProvisioning_AkNano(uint8_t *client_key, uint8_t *client_certificate);
int aknano_provision_device();
status_t aknano_store_provisioning_data(const char *uuid, const char *serial, char *cert_buf, const char *key_buf);
#endif

bool is_valid_certificate_available(bool);

int aknano_provision_tuf_root(struct aknano_context *aknano_context);

#ifdef AKNANO_DELETE_PROVISIONED_TUF_ROOT
int aknano_clear_provisioned_tuf_root();
#endif

#endif /* __AKNANO_PRIV_H__ */
