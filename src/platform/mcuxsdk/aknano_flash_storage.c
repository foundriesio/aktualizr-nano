/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include "mflash_common.h"
#include "mflash_drv.h"

#include "mcuboot_app_support.h"

#include "aknano.h"
#include "aknano_debug.h"


status_t aknano_init_flash_storage()
{
    int mflash_result = mflash_drv_init();

    if (mflash_result != 0) {
        LogError(("mflash_drv_init error %d", mflash_result));
        return -1;
    }
    return 0;
}

status_t aknano_read_flash_storage(int offset, void *output, size_t outputMaxLen)
{
    int mflash_result = mflash_drv_read(AKNANO_STORAGE_FLASH_OFFSET + offset, output, outputMaxLen / 4 * 4);

    if (mflash_result != 0) {
        LogError(("aknano_read_flash_storage: mflash_drv_read error %d", mflash_result));
        return -1;
    }
    return 0;
}

/* offset needs to be aligned to MFLASH_SECTOR_SIZE (4K) */
status_t aknano_write_data_to_flash(int offset, const void *data, size_t data_len)
{
    size_t total_processed = 0;
    size_t chunk_len;
    unsigned char page_buffer[MFLASH_PAGE_SIZE];
    int32_t chunk_flash_addr = offset;
    int32_t mflash_result;
    int32_t next_erase_addr = offset;
    status_t ret = 0;

    do {
        /* The data is expected for be received by page sized chunks (except for the last one) */
        int remaining_bytes = data_len - total_processed;
        if (remaining_bytes < MFLASH_PAGE_SIZE)
            chunk_len = remaining_bytes;
        else
            chunk_len = MFLASH_PAGE_SIZE;

        memcpy(page_buffer, data + total_processed, chunk_len);

        if (chunk_len > 0) {
            /* Perform erase when encountering next sector */
            if (chunk_flash_addr >= next_erase_addr) {
                mflash_result = mflash_drv_sector_erase(next_erase_addr);
                if (mflash_result != 0) {
                    LogError(("store_update_image: Error erasing sector %ld", mflash_result));
                    ret = -2;
                    break;
                }
                next_erase_addr += MFLASH_SECTOR_SIZE;
            }

            /* Clear the unused portion of the buffer (applicable to the last chunk) */
            if (chunk_len < MFLASH_PAGE_SIZE)
                memset((uint8_t *)page_buffer + chunk_len, 0xFF, MFLASH_PAGE_SIZE - chunk_len);

            /* Program the page */
            mflash_result = mflash_drv_page_program(chunk_flash_addr, (uint32_t *)page_buffer);
            if (mflash_result != 0) {
                LogError(("store_update_image: Error storing page %ld", mflash_result));
                ret = -1;
                break;
            }

            total_processed += chunk_len;
            chunk_flash_addr += chunk_len;
        }
    } while (chunk_len == MFLASH_PAGE_SIZE);

    return ret;
}

status_t aknano_write_data_to_storage(int offset, const void *data, size_t data_len)
{
    return aknano_write_data_to_flash(AKNANO_STORAGE_FLASH_OFFSET + offset, data, data_len);
}

void aknano_update_settings_in_flash(struct aknano_settings *aknano_settings)
{
    char flashPageBuffer[256];
    size_t offset = 0;

    memset(flashPageBuffer, 0, sizeof(flashPageBuffer));


    memcpy(flashPageBuffer + offset, &aknano_settings->last_applied_version, sizeof(int));
    offset += sizeof(int);

    memcpy(flashPageBuffer + offset, &aknano_settings->last_confirmed_version, sizeof(int));
    offset += sizeof(int);

    memcpy(flashPageBuffer + offset, aknano_settings->ongoing_update_correlation_id,
           sizeof(aknano_settings->ongoing_update_correlation_id));
    offset += sizeof(aknano_settings->ongoing_update_correlation_id);

    memcpy(flashPageBuffer + offset, &aknano_settings->rollback_retry_count,
           sizeof(aknano_settings->rollback_retry_count));
    offset += sizeof(aknano_settings->rollback_retry_count);

    memcpy(flashPageBuffer + offset, &aknano_settings->rollback_next_retry_time,
           sizeof(aknano_settings->rollback_next_retry_time));
    offset += sizeof(aknano_settings->rollback_next_retry_time);

#ifdef AKNANO_ENABLE_EXPLICIT_REGISTRATION
    flashPageBuffer[offset] = aknano_settings->is_device_registered;
    offset += 1;
#endif

    LogInfo(("Writing settings to flash..."));
    aknano_write_data_to_storage(AKNANO_FLASH_OFF_STATE_BASE, flashPageBuffer, sizeof(flashPageBuffer));
}

status_t aknano_set_image_ready_for_test()
{
    return bl_update_image_state(kSwapType_ReadyForTest);
}

bool aknano_verify_image(size_t image_size)
{
#ifdef AKNANO_DRY_RUN
    return true;
#else
    partition_t update_partition;
    if (bl_get_update_partition_info(&update_partition) != kStatus_Success) {
        /* Could not get update partition info */
        LogError(("Could not get update partition info"));
        return false;
    }
    LogInfo(("Validating image of size %d", image_size));

    if (bl_verify_image((void *)update_partition.start, image_size) <= 0) {
        /* Image validation failed */
        LogError(("Image validation failed"));
        return false;
    } else {
        LogInfo(("Image validation succeeded"));
        return true;
    }
#endif
}

// TODO: interface could be better
void aknano_set_image_confirmed(struct aknano_settings *aknano_settings)
{
    if (!aknano_settings->is_image_permanent) {
        LogInfo((ANSI_COLOR_GREEN "Marking image as Permanent" ANSI_COLOR_RESET));
        if (bl_update_image_state(kSwapType_Permanent) == kStatus_Success)
            aknano_settings->is_image_permanent = true;
        else
            LogError((ANSI_COLOR_RED "Error marking image as Permanent" ANSI_COLOR_RESET));
    }
}

uint32_t aknano_get_target_slot_address(uint8_t current_image_position)
{
    if (current_image_position == 0x01)
        return FLASH_AREA_IMAGE_2_OFFSET;
    else if (current_image_position == 0x02)
        return FLASH_AREA_IMAGE_1_OFFSET;
    else
        return FLASH_AREA_IMAGE_2_OFFSET;
}

bool aknano_is_current_image_permanent()
{
    uint32_t current_status;
    bool is_image_permanent = false;

    aknano_delay(200);
    if (bl_get_image_state(&current_status) == kStatus_Success) {
        if (current_status == kSwapType_Testing) {
            LogInfo((ANSI_COLOR_GREEN "Current image state is Testing" ANSI_COLOR_RESET));
        } else if (current_status == kSwapType_ReadyForTest) {
            LogInfo((ANSI_COLOR_GREEN "Current image state is ReadyForTest" ANSI_COLOR_RESET));
        } else {
            LogInfo((ANSI_COLOR_GREEN "Current image state is Permanent" ANSI_COLOR_RESET));
            is_image_permanent = true;
        }
    } else {
        LogWarn((ANSI_COLOR_RED "Error getting image state"));
    }
    return is_image_permanent;
}

void aknano_get_current_version(uint32_t *running_version, int image_position)
{
    bl_get_image_build_num(running_version, image_position);
}

int aknano_get_image_position()
{
    return get_active_image() + 1;
}

#ifdef AKNANO_ALLOW_PROVISIONING
static status_t aknano_clear_flash_sector(int offset)
{
    int mflash_result = mflash_drv_sector_erase(AKNANO_STORAGE_FLASH_OFFSET + offset);

    if (mflash_result != 0)
        LogError(("EraseSector error %d", mflash_result));

    return 0;
}

/* Data needs to be a 256 bytes array */
static status_t aknano_write_flash_page(int offset, void *data)
{
    int mflash_result = mflash_drv_page_program(AKNANO_STORAGE_FLASH_OFFSET + offset, data);

    if (mflash_result != 0)
        LogError(("aknano_write_flash_page error %d", mflash_result));
    return 0;
}

int aknano_clear_provisioned_data()
{
    int offset;

    LogInfo(("Clearing provisioned device data from flash"));
    for (offset = 0; offset < AKNANO_FLASH_SECTORS_COUNT * MFLASH_SECTOR_SIZE; offset += MFLASH_SECTOR_SIZE)
        aknano_clear_flash_sector(offset);
    return 0;
}

status_t aknano_save_uuid_and_serial(const char *uuid, const char *serial)
{
    char uuid_and_serial[256];

    aknano_clear_provisioned_data();
    memcpy(uuid_and_serial, uuid, AKNANO_MAX_UUID_LENGTH);
    memcpy(uuid_and_serial + 128, serial, AKNANO_MAX_SERIAL_LENGTH);
    aknano_write_flash_page(AKNANO_FLASH_OFF_DEV_UUID, uuid_and_serial);
    return 0;
}

#endif
