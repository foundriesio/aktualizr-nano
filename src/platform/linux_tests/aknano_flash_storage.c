/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include "aknano.h"
#include "aknano_debug.h"

status_t aknano_init_flash_storage()
{
    return 0;
}

status_t aknano_read_flash_storage(int offset, void *output, size_t outputMaxLen)
{
    return 0;
}

/* offset needs to be aligned to MFLASH_SECTOR_SIZE (4K) */
status_t aknano_write_data_to_flash(int offset, const void *data, size_t data_len)
{
    return 0;
}

status_t aknano_write_data_to_storage(int offset, const void *data, size_t data_len)
{
    return aknano_write_data_to_flash(AKNANO_STORAGE_FLASH_OFFSET + offset, data, data_len);
}

void aknano_update_settings_in_flash(struct aknano_settings *aknano_settings)
{

    LogInfo(("Writing settings to flash..."));
}

status_t aknano_set_image_ready_for_test()
{
    return 0;
}

bool aknano_verify_image(size_t image_size)
{
    return true;
}

void aknano_set_image_confirmed(struct aknano_settings *aknano_settings)
{
}

uint32_t aknano_get_target_slot_address(uint8_t current_image_position)
{
    return 0;
}

void aknano_get_current_image_state(struct aknano_settings *aknano_settings)
{
    uint32_t currentStatus;
    bool is_image_permanent = false;

    aknano_delay(200);
    // if (true) {
    //     if (true) {
    //         LogInfo((ANSI_COLOR_GREEN "Current image state is Testing" ANSI_COLOR_RESET));
    //     } else if (false) {
    //         LogInfo((ANSI_COLOR_GREEN "Current image state is ReadyForTest" ANSI_COLOR_RESET));
    //     } else {
    //         LogInfo((ANSI_COLOR_GREEN "Current image state is Permanent" ANSI_COLOR_RESET));
    //         is_image_permanent = true;
    //     }
    // } else {
    //     LogWarn((ANSI_COLOR_RED "Error getting image state"));
    // }

    aknano_settings->is_image_permanent = true;
}

void aknano_get_current_version(uint32_t *running_version, int image_position)
{
    *running_version = 1000;
}

int aknano_get_image_position()
{
    return 1;
}


/* TODO: Improve settings API */
void UpdateSettingValue(const char* name, int value)
{
}

#ifdef AKNANO_ALLOW_PROVISIONING
static status_t aknano_clear_flash_sector(int offset)
{
    return 0;
}

/* Data needs to be a 256 bytes array */
static status_t aknano_write_flash_page(int offset, void *data)
{
    return 0;
}

int aknano_clear_provisioned_data()
{
    return 0;
}

status_t aknano_save_uuid_and_serial(const char *uuid, const char *serial)
{
    return 0;
}

#endif
