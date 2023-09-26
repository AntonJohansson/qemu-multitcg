/*
 * TriCore cpu parameters for qemu.
 *
 * Copyright (c) 2012-2014 Bastian Koppelmann C-Lab/University Paderborn
 * SPDX-License-Identifier: LGPL-2.1+
 */

#ifndef TRICORE_CPU_PARAM_H
#define TRICORE_CPU_PARAM_H

#define TARGET_LONG_BITS 32
#define TARGET_PHYS_ADDR_SPACE_BITS 32
#define TARGET_VIRT_ADDR_SPACE_BITS 32

#ifdef CONFIG_USER_ONLY
# define TARGET_PAGE_BITS 14
#else
# define TARGET_PAGE_BITS_VARY
# define TARGET_PAGE_BITS_MIN_SPECIFIC 14
#endif

#endif
