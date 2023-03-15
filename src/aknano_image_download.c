/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define LIBRARY_LOG_LEVEL LOG_INFO


#include "mbedtls/sha256.h"

#include "aknano_priv.h"
#include "flexspi_flash_config.h"

#include <stdio.h>
#include <time.h>

#define AKNANO_REQUEST_BODY ""
#define AKNANO_REQUEST_BODY_LEN sizeof(AKNANO_REQUEST_BODY) - 1
#define AKNANO_DOWNLOAD_ENDPOINT AKNANO_FACTORY_UUID ".ostree.foundries.io"
#define AKNANO_DOWNLOAD_PORT AKNANO_DEVICE_GATEWAY_PORT


/**
 * @brief The HTTP status code returned for partial content.
 */
#define httpexampleHTTP_STATUS_CODE_PARTIAL_CONTENT          206



#include "aknano_secret.h"
static const char downloadServer_ROOT_CERTIFICATE_PEM[] = AKNANO_DEVICE_GATEWAY_CERTIFICATE;

#define AKNANO_DOWNLOAD_ENDPOINT_LEN sizeof(AKNANO_DOWNLOAD_ENDPOINT) - 1

static const uint32_t donwloadServer_ROOT_CERTIFICATE_LENGTH = sizeof(downloadServer_ROOT_CERTIFICATE_PEM);


BaseType_t aknano_connect_to_download_server(struct aknano_network_context *network_context)
{
    BaseType_t ret;

    init_network_context(network_context);
    ret = aknano_mtls_connect(network_context,
                              AKNANO_DOWNLOAD_ENDPOINT,
                              AKNANO_DOWNLOAD_ENDPOINT_LEN,
                              AKNANO_DOWNLOAD_PORT,
                              downloadServer_ROOT_CERTIFICATE_PEM,
                              donwloadServer_ROOT_CERTIFICATE_LENGTH);

    if (ret != pdPASS) {
        /* Log error to indicate connection failure after all
         * reconnect attempts are over. */
        LogError(("Failed to connect to HTTP download server"));
        return pdFAIL;
    }
    return pdPASS;
}

static int HandleReceivedData(const unsigned char *data, int offset, int data_len, int partition_phys_addr)
{
#ifdef AKNANO_DRY_RUN
    LogInfo(("** Dry run mode, skipping flash operations"));
    return 0;
#endif
    int32_t retval = 0;

    LogInfo(("Writing image chunk to flash. offset=%d len=%d", offset, data_len));

    if (offset + data_len >= AKNANO_MAX_FIRMWARE_SIZE) {
        LogError(("store_update_image: AKNANO_MAX_FIRMWARE_SIZE boundary exceeded"));
        retval = -1;
        return -1;
    }

    retval = aknano_write_data_to_flash(partition_phys_addr + offset, data, data_len);
    return retval;
}


static BaseType_t aknano_download_image(
    struct aknano_network_context *network_context,
    const char *                   pcPath,
    struct aknano_context *        aknano_context)
{
    BaseType_t xStatus = pdPASS;

    uint8_t image_position = aknano_context->settings->image_position;

    /* The size of the file we are trying to download . */
    size_t xFileSize = 0;

    /* The number of bytes we want to request with in each range of the file
     * bytes. */
    size_t xNumReqBytes = 0;
    /* xCurByte indicates which starting byte we want to download next. */
    size_t xCurByte = 0;
    uint32_t dst_partition_phys_addr;
    uint8_t sha256_bytes[AKNANO_SHA256_LEN];

    if (image_position == 0x01)
        dst_partition_phys_addr = FLASH_AREA_IMAGE_2_OFFSET;
    else if (image_position == 0x02)
        dst_partition_phys_addr = FLASH_AREA_IMAGE_1_OFFSET;
    else
        dst_partition_phys_addr = FLASH_AREA_IMAGE_2_OFFSET;

    configASSERT(pcPath != NULL);

    xNumReqBytes = AKNANO_IMAGE_DOWNLOAD_REQUEST_LENGTH;
    xStatus = pdPASS;

    int32_t stored = 0;

    /* Initialize SHA256 calculation for FW image */
    mbedtls_sha256_init(&aknano_context->sha256_context);
    mbedtls_sha256_starts(&aknano_context->sha256_context, 0);

    xFileSize = aknano_context->selected_target.expected_size;
    /* Here we iterate sending byte range requests until the full file has been
     * downloaded. We keep track of the next byte to download with xCurByte, and
     * increment by xNumReqBytes after each iteration. When xCurByte reaches
     * xFileSize, we stop downloading. */
    while ((xStatus == pdPASS) && (xCurByte < xFileSize)) {
        LogInfo((ANSI_COLOR_GREEN "Downloading new image. Retrieving bytes %d-%d of %d" ANSI_COLOR_RESET,
                 (int)(xCurByte),
                 (int)(xCurByte + xNumReqBytes - 1),
                 (int)xFileSize));
        xStatus = aknano_mtls_send_http_request(
            network_context,
            AKNANO_DOWNLOAD_ENDPOINT,
            AKNANO_DOWNLOAD_ENDPOINT_LEN,
            HTTP_METHOD_GET,
            pcPath,
            "",
            0,
            ucUserBuffer,
            sizeof(ucUserBuffer),
            NULL,
            NULL,
            0,
            xCurByte,
            xCurByte + xNumReqBytes - 1
            );

        if (xStatus == pdPASS) {
            mbedtls_sha256_update(&aknano_context->sha256_context, network_context->reply_body, network_context->reply_body_len);
            if (HandleReceivedData(network_context->reply_body, xCurByte, network_context->reply_body_len, dst_partition_phys_addr) < 0) {
                LogError(("Error during HandleReceivedData"));
                xStatus = pdFAIL;
                break;
            }

            stored += network_context->reply_body_len;

            /* We increment by the content length because the server may not
             * have sent us the range we requested. */
            xCurByte += network_context->reply_body_len;

            if ((xFileSize - xCurByte) < xNumReqBytes)
                xNumReqBytes = xFileSize - xCurByte;

            xStatus = (network_context->reply_http_code == httpexampleHTTP_STATUS_CODE_PARTIAL_CONTENT) ? pdPASS : pdFAIL;
        }
    }
    LogInfo(("Disconnecting"));
    aknano_mtls_disconnect(network_context);

    if (stored != aknano_context->selected_target.expected_size) {
        LogInfo((ANSI_COLOR_MAGENTA "Actual file size (%ld bytes) does not match expected size (%ld bytes)" ANSI_COLOR_RESET, stored, aknano_context->selected_target.expected_size));
        xStatus = pdFAIL;
    } else {
        LogInfo((ANSI_COLOR_MAGENTA "Actual file size (%ld bytes) matches expected size (%ld bytes)" ANSI_COLOR_RESET, stored, aknano_context->selected_target.expected_size));
    }

    if (xStatus == pdPASS) {
        mbedtls_sha256_finish_ret(&aknano_context->sha256_context, sha256_bytes);
        if (memcmp(&aknano_context->selected_target.expected_hash, sha256_bytes, sizeof(sha256_bytes))) {
            LogInfo((ANSI_COLOR_RED "Downloaded image SHA256 does not match the expected value" ANSI_COLOR_RESET));
            return false;
        } else {
            LogInfo((ANSI_COLOR_MAGENTA "Downloaded image SHA256 matches the expected value" ANSI_COLOR_RESET));
        }

#ifndef AKNANO_DRY_RUN
        partition_t update_partition;
        if (bl_get_update_partition_info(&update_partition) != kStatus_Success) {
            /* Could not get update partition info */
            LogError(("Could not get update partition info"));
            return pdFAIL;
        }
        LogInfo(("Validating image of size %d", stored));

        struct image_header *ih;
        ih = (struct image_header *)update_partition.start;
        if (bl_verify_image((void *)update_partition.start, stored) <= 0) {
            /* Image validation failed */
            LogError(("Image validation failed magic=0x%X", ih->ih_magic));
            return false;
        } else {
            LogInfo(("Image validation succeeded"));
            return true;
        }
#endif
    } else {
        return false;
    }
}

int aknano_download_and_flash_image(struct aknano_context *aknano_context)
{
    BaseType_t xDemoStatus = pdPASS;
    struct aknano_network_context aknano_network_context;

    xDemoStatus = aknano_connect_to_download_server(&aknano_network_context);

    if (xDemoStatus == pdPASS) {
        uint8_t *h = aknano_context->selected_target.expected_hash;
        char relative_path[AKNANO_MAX_URI_LENGTH];
        sprintf(relative_path, "/mcu/files/"
                "%02x/%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                "%02x%02x%02x%02x%02x%02x%02x%02x"
                ".bin",
                *(h + 0), *(h + 1), *(h + 2), *(h + 3), *(h + 4), *(h + 5), *(h + 6), *(h + 7),
                *(h + 8), *(h + 9), *(h + 10), *(h + 11), *(h + 12), *(h + 13), *(h + 14), *(h + 15),
                *(h + 16), *(h + 17), *(h + 18), *(h + 19), *(h + 20), *(h + 21), *(h + 22), *(h + 23),
                *(h + 24), *(h + 25), *(h + 26), *(h + 27), *(h + 28), *(h + 29), *(h + 30), *(h + 31));
        LogInfo(("Download relative path=%s", relative_path));
        xDemoStatus = aknano_download_image(&aknano_network_context, relative_path, aknano_context);
    }

    /**************************** Disconnect. ******************************/

    /* Close the network connection to clean up any system resources that the
     * demo may have consumed. */
    return xDemoStatus;
}
