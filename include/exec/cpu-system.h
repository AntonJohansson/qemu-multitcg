/*
 * defines common to all virtual CPUs
 *
 *  Copyright (c) 2003 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CPU_SYSTEM_H
#define CPU_SYSTEM_H

#ifdef CONFIG_USER_ONLY
#error cpu-system.h included from non system mode code
#endif

/* MMU memory access macros */

#include "exec/hwaddr.h"

#define SUFFIX
#define ARG1         as
#define ARG1_DECL    AddressSpace *as
#define TARGET_ENDIANNESS
#include "exec/memory_ldst.h.inc"

#define SUFFIX       _cached_slow
#define ARG1         cache
#define ARG1_DECL    MemoryRegionCache *cache
#define TARGET_ENDIANNESS
#include "exec/memory_ldst.h.inc"

static inline void stl_phys_notdirty(AddressSpace *as, hwaddr addr, uint32_t val)
{
    address_space_stl_notdirty(as, addr, val,
                               MEMTXATTRS_UNSPECIFIED, NULL);
}

#define SUFFIX
#define ARG1         as
#define ARG1_DECL    AddressSpace *as
#define TARGET_ENDIANNESS
#include "exec/memory_ldst_phys.h.inc"

/* Inline fast path for direct RAM access.  */
#define ENDIANNESS
#include "exec/memory_ldst_cached.h.inc"

#define SUFFIX       _cached
#define ARG1         cache
#define ARG1_DECL    MemoryRegionCache *cache
#define TARGET_ENDIANNESS
#include "exec/memory_ldst_phys.h.inc"

#endif /* CPU_SYSTEM_H */
