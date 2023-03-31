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
void aknano_init_settings(struct aknano_settings *aknano_settings);
void aknano_init_context(struct aknano_context *aknano_context, struct aknano_settings *aknano_settings);
void aknano_send_installation_finished_event(struct aknano_settings *aknano_settings);
void aknano_set_image_confirmed(struct aknano_settings *aknano_settings);
void aknano_init(struct aknano_settings *aknano_settings);

void aknano_sample_loop(uint32_t *remaining_iterations);
void aknano_reboot_command();

#endif
