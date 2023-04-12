/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_PUBLIC_API_H__
#define __AKNANO_PUBLIC_API_H__

#include "aknano.h"

struct aknano_context;

/* API */
bool aknano_has_matching_target(struct aknano_context *aknano_context);
bool aknano_install_selected_target(struct aknano_context *aknano_context);
bool aknano_is_rollback(struct aknano_context *aknano_context);
bool aknano_is_temp_image(struct aknano_settings *aknano_settings);
bool aknano_set_application_self_test_ok(struct aknano_settings *aknano_settings);
bool aknano_should_retry_rollback(struct aknano_context *aknano_context);
int aknano_checkin(struct aknano_context *aknano_context);
int aknano_get_setting(struct aknano_context *aknano_context, const char *setting_name);
int aknano_limit_sleep_time_range(int sleep_time);
uint32_t aknano_get_current(struct aknano_context *aknano_context);
uint32_t aknano_get_selected_version(struct aknano_context *aknano_context);
void aknano_init_context(struct aknano_context *aknano_context, struct aknano_settings *aknano_settings);
void aknano_init_settings(struct aknano_settings *aknano_settings);
void aknano_init(struct aknano_settings *aknano_settings);
void aknano_send_installation_finished_event(struct aknano_settings *aknano_settings);
void aknano_set_image_confirmed(struct aknano_settings *aknano_settings);


void aknano_sample_loop(uint32_t *remaining_iterations);
void aknano_reboot_command();

#endif
