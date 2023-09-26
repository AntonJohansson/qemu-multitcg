/*
 * m68k cpu parameters for qemu.
 *
 * Copyright (c) 2005-2007 CodeSourcery
 * SPDX-License-Identifier: LGPL-2.0+
 */

#ifndef M68K_CPU_PARAM_H
#define M68K_CPU_PARAM_H

#define TARGET_LONG_BITS 32
/*
 * Coldfire Linux uses 8k pages
 * and m68k linux uses 4k pages
 * use the smallest one
 */
#ifdef CONFIG_USER_ONLY
#define TARGET_PAGE_BITS 12
#else
#define TARGET_PAGE_BITS_VARY
#define TARGET_PAGE_BITS_MIN_SPECIFIC 12
#endif

#define TARGET_PHYS_ADDR_SPACE_BITS 32
#define TARGET_VIRT_ADDR_SPACE_BITS 32

#endif
