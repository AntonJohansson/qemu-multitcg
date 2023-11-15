/*
 * QEMU Xen support
 *
 * This work is licensed under the terms of the GNU GPL, version 2 or later.
 * See the COPYING file in the top-level directory.
 */

/* header to be included in non-Xen-specific code */

#ifndef SYSEMU_XEN_H
#define SYSEMU_XEN_H

#include "exec/cpu-common.h"

extern bool xen_allowed;

#define xen_enabled()           (xen_allowed)

#ifndef CONFIG_USER_ONLY
void xen_hvm_modified_memory(ram_addr_t start, ram_addr_t length);
void xen_ram_alloc(ram_addr_t ram_addr, ram_addr_t size,
                   struct MemoryRegion *mr, Error **errp);
#endif

#endif
