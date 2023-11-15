/*
 * Copyright (C) 2014       Citrix Systems UK Ltd.
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

#include "qemu/osdep.h"
#include "sysemu/xen.h"
#include "qapi/qapi-commands-migration.h"

bool xen_allowed;

void qmp_xen_set_global_dirty_log(bool enable, Error **errp)
{
}

#ifndef CONFIG_USER_ONLY
void xen_hvm_modified_memory(ram_addr_t start, ram_addr_t length)
{
    /* nothing */
}
void xen_ram_alloc(ram_addr_t ram_addr, ram_addr_t size,
                   MemoryRegion *mr, Error **errp)
{
    g_assert_not_reached();
}
#endif
