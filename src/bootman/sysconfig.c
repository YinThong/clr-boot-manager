/*
 * This file is part of clr-boot-manager.
 *
 * Copyright © 2016-2017 Intel Corporation
 *
 * clr-boot-manager is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "bootman.h"
#include "bootman_private.h"
#include "files.h"
#include "log.h"

void cbm_free_sysconfig(SystemConfig *config)
{
        if (!config) {
                return;
        }
        free(config->prefix);
        free(config->boot_device);
        cbm_probe_free(config->root_device);
        free(config);
}

SystemConfig *cbm_inspect_root(const char *path)
{
        if (!path) {
                return NULL;
        }
        SystemConfig *c = NULL;
        char *realp = NULL;
        char *boot = NULL;
        char *rel = NULL;

        realp = realpath(path, NULL);
        if (!realp) {
                LOG_ERROR("Path specified does not exist: %s", path);
                return NULL;
        }

        c = calloc(1, sizeof(struct SystemConfig));
        if (!c) {
                DECLARE_OOM();
                free(realp);
                return NULL;
        }
        c->prefix = realp;
        c->wanted_boot_mask = 0;

        /* Find legacy relative to root, on GPT */
        boot = get_legacy_boot_device(realp);
        if (boot) {
                c->boot_device = boot;
                c->wanted_boot_mask |= BOOTLOADER_CAP_LEGACY;
                LOG_INFO("Discovered legacy boot device: %s", boot);
        } else {
                /* Discover UEFI boot */
                c->boot_device = get_boot_device();
                c->wanted_boot_mask |= BOOTLOADER_CAP_UEFI;
        }

        /* Our probe methods are GPT only. If we found one, it's definitely GPT */
        if (c->boot_device) {
                rel = realpath(c->boot_device, NULL);
                if (!rel) {
                        LOG_FATAL("Cannot determine boot device: %s %s",
                                  c->boot_device,
                                  strerror(errno));
                } else {
                        free(c->boot_device);
                        c->boot_device = rel;
                        LOG_INFO("Discovered boot device: %s", rel);
                }
                c->wanted_boot_mask |= BOOTLOADER_CAP_GPT;
        } else {
                /* Legacy boot, non-GPT */
                c->wanted_boot_mask = BOOTLOADER_CAP_LEGACY;
        }

        c->root_device = cbm_probe_path(realp);

        return c;
}

bool cbm_is_sysconfig_sane(SystemConfig *config)
{
        if (!config) {
                LOG_FATAL("sysconfig insane: Missing config");
                return false;
        }
        if (!config->root_device) {
                LOG_FATAL("sysconfig insane: Missing root device");
                return false;
        }
        return true;
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: nil
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 expandtab:
 * :indentSize=8:tabSize=8:noTabs=true:
 */
