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
#ifndef CPU_ALL_H
#define CPU_ALL_H

#include "exec/cpu-common.h"
#include "exec/memory.h"
#include "exec/tswap.h"
#include "qemu/thread.h"
#include "hw/core/cpu.h"
#include "qemu/rcu.h"

/* some important defines:
 *
 * HOST_BIG_ENDIAN : whether the host cpu is big endian and
 * otherwise little endian.
 *
 * TARGET_BIG_ENDIAN : same for the target cpu
 */

#if HOST_BIG_ENDIAN != TARGET_BIG_ENDIAN
#define BSWAP_NEEDED
#endif

#if TARGET_LONG_SIZE == 4
#define tswapl(s) tswap32(s)
#define tswapls(s) tswap32s((uint32_t *)(s))
#define bswaptls(s) bswap32s(s)
#else
#define tswapl(s) tswap64(s)
#define tswapls(s) tswap64s((uint64_t *)(s))
#define bswaptls(s) bswap64s(s)
#endif

/* Target-endianness CPU memory access functions. These fit into the
 * {ld,st}{type}{sign}{size}{endian}_p naming scheme described in bswap.h.
 */
#if TARGET_BIG_ENDIAN
#define lduw_p(p) lduw_be_p(p)
#define ldsw_p(p) ldsw_be_p(p)
#define ldl_p(p) ldl_be_p(p)
#define ldq_p(p) ldq_be_p(p)
#define stw_p(p, v) stw_be_p(p, v)
#define stl_p(p, v) stl_be_p(p, v)
#define stq_p(p, v) stq_be_p(p, v)
#define ldn_p(p, sz) ldn_be_p(p, sz)
#define stn_p(p, sz, v) stn_be_p(p, sz, v)
#else
#define lduw_p(p) lduw_le_p(p)
#define ldsw_p(p) ldsw_le_p(p)
#define ldl_p(p) ldl_le_p(p)
#define ldq_p(p) ldq_le_p(p)
#define stw_p(p, v) stw_le_p(p, v)
#define stl_p(p, v) stl_le_p(p, v)
#define stq_p(p, v) stq_le_p(p, v)
#define ldn_p(p, sz) ldn_le_p(p, sz)
#define stn_p(p, sz, v) stn_le_p(p, sz, v)
#endif

/* 
 * Check that softmmu targets are using variable page sizes, we need this
 * for the TARGET_PAGE_* macros to be target independent.
 */
#if !defined(TARGET_PAGE_BITS_VARY)
# error Need to use TARGET_PAGE_BITS_VARY for target-independent page macros
#endif

#if defined(CONFIG_BSD) && defined(CONFIG_USER_ONLY)
/* FIXME: Code that sets/uses this is broken and needs to go away.  */
#define PAGE_RESERVED  0x0100
#endif

CPUArchState *cpu_copy(CPUArchState *env);

/* Validate correct placement of CPUArchState. */
QEMU_BUILD_BUG_ON(offsetof(ArchCPU, parent_obj) != 0);
QEMU_BUILD_BUG_ON(offsetof(ArchCPU, env) != sizeof(CPUState));

#endif /* CPU_ALL_H */
