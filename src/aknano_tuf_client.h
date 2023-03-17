/*
 * Copyright 2021-2023 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __AKNANO_TUF_CLIENT_H__
#define __AKNANO_TUF_CLIENT_H__

int aknano_provision_tuf_root(struct aknano_context *aknano_context);

#ifdef AKNANO_DELETE_PROVISIONED_TUF_ROOT
int aknano_clear_provisioned_tuf_root();
#endif

#endif
