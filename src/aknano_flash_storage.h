/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_FLASH_STORAGE_H__
#define __AKNANO_FLASH_STORAGE_H__

#include <stddef.h>

#include "aknano.h"

void aknano_update_settings_in_flash(struct aknano_settings *aknano_settings);

status_t aknano_init_flash_storage();
status_t aknano_read_flash_storage(int offset, void *output, size_t outputMaxLen);
status_t aknano_write_data_to_flash(int offset, const void *data, size_t data_len);
status_t aknano_write_data_to_storage(int offset, const void *data, size_t data_len);

status_t aknano_set_image_ready_for_test();
void aknano_set_image_confirmed(struct aknano_settings *aknano_settings);
void aknano_get_current_image_state(struct aknano_settings *aknano_settings);

bool aknano_verify_image(size_t image_size);

uint32_t aknano_get_target_slot_address(uint8_t current_image_position);

void aknano_get_current_version(uint32_t *running_version, int image_position);
int aknano_get_image_position();

#ifdef AKNANO_ALLOW_PROVISIONING
status_t aknano_save_uuid_and_serial(const char *uuid, const char *serial);
#endif

#endif
