/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifdef AKNANO_ALLOW_PROVISIONING
#define LIBRARY_LOG_LEVEL LOG_INFO

#include "aknano_secret.h"
#include "aknano_provisioning_secret.h"
#include "aknano_priv.h"

int aknano_gen_device_certificate_and_key(const char *uuid, const char *factory_name, const char *serial_string, unsigned char *cert_buf, unsigned char *key_buf);

#ifndef AKNANO_ENABLE_EL2GO
static unsigned char cert_buf[AKNANO_CERT_BUF_SIZE];
static unsigned char key_buf[AKNANO_CERT_BUF_SIZE];
#endif

int aknano_provision_device()
{
    /*
     * When EdgeLock 2GO is used, this function only generates the device serial
     *  and UUID
     */

    int ret = 0;

#ifndef AKNANO_ENABLE_EL2GO
    int offset;
#endif
    char uuid[AKNANO_MAX_UUID_LENGTH];
    char serial[AKNANO_MAX_SERIAL_LENGTH];
    // char temp_buf[257];

    aknano_gen_serial_and_uuid(uuid, serial);
    LogInfo(("uuid=%s", uuid));
    LogInfo(("serial=%s", serial));

#ifndef AKNANO_ENABLE_EL2GO
    ret = aknano_gen_device_certificate_and_key(uuid, AKNANO_FACTORY_NAME,
                                                serial, cert_buf, key_buf);
    LogInfo(("aknano_gen_random_device_certificate_and_key ret=%d", ret));
    LogInfo(("cert_buf:\r\n%s", cert_buf));
    LogInfo(("key_buf:\r\n%s", key_buf));

    if (ret != 0) {
        LogError(("Certificate generation error %d", ret));
        return ret;
    }
#endif

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_UUID, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("BEFORE uuid=%s", temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_SERIAL, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("BEFORE serial=%s", temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_CERTIFICATE, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("BEFORE cert=%s", temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_CERTIFICATE, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("BEFORE cert=%s", temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_KEY, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("BEFORE key=%s", temp_buf));

    // Save Cert, Key, UUID and Serial to flash

#ifdef AKNANO_ENABLE_EL2GO
    aknano_store_provisioning_data(uuid, serial, NULL, NULL);
#else
    aknano_store_provisioning_data(uuid, serial, cert_buf, key_buf);
#endif

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_CERTIFICATE, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("AFTER [%x] cert=%s", temp_buf[0], temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_CERTIFICATE, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("AFTER cert=%s", temp_buf));

    // aknano_read_flash_storage(AKNANO_FLASH_OFF_DEV_KEY, temp_buf, 256);
    // temp_buf[256] = 0;
    // LogInfo(("AFTER key=%s", temp_buf));

    // aknano_read_flash_storage(AKNANO_FLASH_OFF_DEV_UUID, temp_buf, 256);
    // temp_buf[256] = 0;
    // LogInfo(("AFTER uuid=%s", temp_buf));

    // aknano_read_flash_storage(AKNANO_FLASH_OFF_DEV_SERIAL, temp_buf, 256);
    // temp_buf[256] = 0;
    // LogInfo(("AFTER serial=%s", temp_buf));

#ifndef AKNANO_ENABLE_EL2GO
#ifdef AKNANO_ENABLE_SE05X
    LogInfo(("Provisioning Key and Certificate using PKCS#11 interface. Using SE05X"));
#else
    LogInfo(("Provisioning Key and Certificate using PKCS#11 interface. Using flash device"));
#endif
    vDevModeKeyProvisioning_AkNano((uint8_t *)key_buf,
                                   (uint8_t *)cert_buf);
    LogInfo(("Provisioning done"));
    vTaskDelay(pdMS_TO_TICKS(2000));
#endif
    return ret;
}
#endif
