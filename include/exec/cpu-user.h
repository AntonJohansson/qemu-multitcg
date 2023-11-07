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
#ifndef CPU_USER_H
#define CPU_USER_H

#ifndef CONFIG_USER_ONLY
#error "cpu-user.h included from non-user code"
#endif

#include "exec/user/abitypes.h"
#include "exec/user/guest-base.h"

/* MMU memory access macros */

extern bool have_guest_base;

/*
 * If non-zero, the guest virtual address space is a contiguous subset
 * of the host virtual address space, i.e. '-R reserved_va' is in effect
 * either from the command-line or by default.  The value is the last
 * byte of the guest address space e.g. UINT32_MAX.
 *
 * If zero, the host and guest virtual address spaces are intermingled.
 */
extern unsigned long reserved_va;

/*
 * Limit the guest addresses as best we can.
 *
 * When not using -R reserved_va, we cannot really limit the guest
 * to less address space than the host.  For 32-bit guests, this
 * acts as a sanity check that we're not giving the guest an address
 * that it cannot even represent.  For 64-bit guests... the address
 * might not be what the real kernel would give, but it is at least
 * representable in the guest.
 *
 * TODO: Improve address allocation to avoid this problem, and to
 * avoid setting bits at the top of guest addresses that might need
 * to be used for tags.
 */
#define GUEST_ADDR_MAX_                                                 \
    ((MIN_CONST(TARGET_VIRT_ADDR_SPACE_BITS, TARGET_ABI_BITS) <= 32) ?  \
     UINT32_MAX : ~0ul)
#define GUEST_ADDR_MAX    (reserved_va ? : GUEST_ADDR_MAX_)

void page_dump(FILE *f);

typedef int (*walk_memory_regions_fn)(void *, target_ulong,
                                      target_ulong, unsigned long);
int walk_memory_regions(void *, walk_memory_regions_fn);

int page_get_flags(target_ulong address);
void page_set_flags(target_ulong start, target_ulong last, int flags);
void page_reset_target_data(target_ulong start, target_ulong last);

/**
 * page_check_range
 * @start: first byte of range
 * @len: length of range
 * @flags: flags required for each page
 *
 * Return true if every page in [@start, @start+@len) has @flags set.
 * Return false if any page is unmapped.  Thus testing flags == 0 is
 * equivalent to testing for flags == PAGE_VALID.
 */
bool page_check_range(target_ulong start, target_ulong last, int flags);

/**
 * page_check_range_empty:
 * @start: first byte of range
 * @last: last byte of range
 * Context: holding mmap lock
 *
 * Return true if the entire range [@start, @last] is unmapped.
 * The memory lock must be held so that the caller will can ensure
 * the result stays true until a new mapping can be installed.
 */
bool page_check_range_empty(target_ulong start, target_ulong last);

/**
 * page_find_range_empty
 * @min: first byte of search range
 * @max: last byte of search range
 * @len: size of the hole required
 * @align: alignment of the hole required (power of 2)
 *
 * If there is a range [x, x+@len) within [@min, @max] such that
 * x % @align == 0, then return x.  Otherwise return -1.
 * The memory lock must be held, as the caller will want to ensure
 * the returned range stays empty until a new mapping can be installed.
 */
target_ulong page_find_range_empty(target_ulong min, target_ulong max,
                                   target_ulong len, target_ulong align);

/**
 * page_get_target_data(address)
 * @address: guest virtual address
 *
 * Return TARGET_PAGE_DATA_SIZE bytes of out-of-band data to associate
 * with the guest page at @address, allocating it if necessary.  The
 * caller should already have verified that the address is valid.
 *
 * The memory will be freed when the guest page is deallocated,
 * e.g. with the munmap system call.
 */
void *page_get_target_data(target_ulong address)
    __attribute__((returns_nonnull));

#endif /* CPU_USER_H */
