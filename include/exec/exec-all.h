/*
 * internal execution defines for qemu
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

#ifndef EXEC_ALL_H
#define EXEC_ALL_H

#include "qemu/typedefs.h"
#include "exec/exec-common.h"
#include "exec/translation-block.h"
#if defined(CONFIG_USER_ONLY)
#include "exec/cpu_ldst.h"
#endif
#include "qemu/clang-tsa.h"

/**
 * probe_access:
 * @env: CPUArchState
 * @addr: guest virtual address to look up
 * @size: size of the access
 * @access_type: read, write or execute permission
 * @mmu_idx: MMU index to use for lookup
 * @retaddr: return address for unwinding
 *
 * Look up the guest virtual address @addr.  Raise an exception if the
 * page does not satisfy @access_type.  Raise an exception if the
 * access (@addr, @size) hits a watchpoint.  For writes, mark a clean
 * page as dirty.
 *
 * Finally, return the host address for a page that is backed by RAM,
 * or NULL if the page requires I/O.
 */
void *probe_access(CPUArchState *env, vaddr addr, int size,
                   MMUAccessType access_type, int mmu_idx, uintptr_t retaddr);

static inline void *probe_write(CPUArchState *env, vaddr addr, int size,
                                int mmu_idx, uintptr_t retaddr)
{
    return probe_access(env, addr, size, MMU_DATA_STORE, mmu_idx, retaddr);
}

static inline void *probe_read(CPUArchState *env, vaddr addr, int size,
                               int mmu_idx, uintptr_t retaddr)
{
    return probe_access(env, addr, size, MMU_DATA_LOAD, mmu_idx, retaddr);
}

/**
 * probe_access_flags:
 * @env: CPUArchState
 * @addr: guest virtual address to look up
 * @size: size of the access
 * @access_type: read, write or execute permission
 * @mmu_idx: MMU index to use for lookup
 * @nonfault: suppress the fault
 * @phost: return value for host address
 * @retaddr: return address for unwinding
 *
 * Similar to probe_access, loosely returning the TLB_FLAGS_MASK for
 * the page, and storing the host address for RAM in @phost.
 *
 * If @nonfault is set, do not raise an exception but return TLB_INVALID_MASK.
 * Do not handle watchpoints, but include TLB_WATCHPOINT in the returned flags.
 * Do handle clean pages, so exclude TLB_NOTDIRY from the returned flags.
 * For simplicity, all "mmio-like" flags are folded to TLB_MMIO.
 */
int probe_access_flags(CPUArchState *env, vaddr addr, int size,
                       MMUAccessType access_type, int mmu_idx,
                       bool nonfault, void **phost, uintptr_t retaddr);

#ifndef CONFIG_USER_ONLY
/**
 * probe_access_full:
 * Like probe_access_flags, except also return into @pfull.
 *
 * The CPUTLBEntryFull structure returned via @pfull is transient
 * and must be consumed or copied immediately, before any further
 * access or changes to TLB @mmu_idx.
 */
int probe_access_full(CPUArchState *env, vaddr addr, int size,
                      MMUAccessType access_type, int mmu_idx,
                      bool nonfault, void **phost,
                      CPUTLBEntryFull **pfull, uintptr_t retaddr);

/**
 * probe_access_mmu() - Like probe_access_full except cannot fault and
 * doesn't trigger instrumentation.
 *
 * @env: CPUArchState
 * @vaddr: virtual address to probe
 * @size: size of the probe
 * @access_type: read, write or execute permission
 * @mmu_idx: softmmu index
 * @phost: ptr to return value host address or NULL
 * @pfull: ptr to return value CPUTLBEntryFull structure or NULL
 *
 * The CPUTLBEntryFull structure returned via @pfull is transient
 * and must be consumed or copied immediately, before any further
 * access or changes to TLB @mmu_idx.
 *
 * Returns: TLB flags as per probe_access_flags()
 */
int probe_access_full_mmu(CPUArchState *env, vaddr addr, int size,
                          MMUAccessType access_type, int mmu_idx,
                          void **phost, CPUTLBEntryFull **pfull);

#endif

/* GETPC is the true target of the return instruction that we'll execute.  */
#if defined(CONFIG_TCG_INTERPRETER)
extern __thread uintptr_t tci_tb_ptr;
# define GETPC() tci_tb_ptr
#else
# define GETPC() \
    ((uintptr_t)__builtin_extract_return_addr(__builtin_return_address(0)))
#endif

/* The true return address will often point to a host insn that is part of
   the next translated guest insn.  Adjust the address backward to point to
   the middle of the call insn.  Subtracting one would do the job except for
   several compressed mode architectures (arm, mips) which set the low bit
   to indicate the compressed mode; subtracting two works around that.  It
   is also the case that there are no host isas that contain a call insn
   smaller than 4 bytes, so we don't worry about special-casing this.  */
#define GETPC_ADJ   2

#if !defined(CONFIG_USER_ONLY)

/**
 * iotlb_to_section:
 * @cpu: CPU performing the access
 * @index: TCG CPU IOTLB entry
 *
 * Given a TCG CPU IOTLB entry, return the MemoryRegionSection that
 * it refers to. @index will have been initially created and returned
 * by memory_region_section_get_iotlb().
 */
struct MemoryRegionSection *iotlb_to_section(CPUState *cpu,
                                             hwaddr index, MemTxAttrs attrs);
#endif

/**
 * get_page_addr_code_hostp()
 * @env: CPUArchState
 * @addr: guest virtual address of guest code
 *
 * See get_page_addr_code() (full-system version) for documentation on the
 * return value.
 *
 * Sets *@hostp (when @hostp is non-NULL) as follows.
 * If the return value is -1, sets *@hostp to NULL. Otherwise, sets *@hostp
 * to the host address where @addr's content is kept.
 *
 * Note: this function can trigger an exception.
 */
tb_page_addr_t get_page_addr_code_hostp(CPUArchState *env, vaddr addr,
                                        void **hostp);

/**
 * get_page_addr_code()
 * @env: CPUArchState
 * @addr: guest virtual address of guest code
 *
 * If we cannot translate and execute from the entire RAM page, or if
 * the region is not backed by RAM, returns -1. Otherwise, returns the
 * ram_addr_t corresponding to the guest code at @addr.
 *
 * Note: this function can trigger an exception.
 */
static inline tb_page_addr_t get_page_addr_code(CPUArchState *env,
                                                vaddr addr)
{
    return get_page_addr_code_hostp(env, addr, NULL);
}

#if defined(CONFIG_USER_ONLY)
void TSA_NO_TSA mmap_lock(void);
void TSA_NO_TSA mmap_unlock(void);
bool have_mmap_lock(void);

static inline void mmap_unlock_guard(void *unused)
{
    mmap_unlock();
}

#define WITH_MMAP_LOCK_GUARD()                                            \
    for (int _mmap_lock_iter __attribute__((cleanup(mmap_unlock_guard)))  \
         = (mmap_lock(), 0); _mmap_lock_iter == 0; _mmap_lock_iter = 1)

/**
 * adjust_signal_pc:
 * @pc: raw pc from the host signal ucontext_t.
 * @is_write: host memory operation was write, or read-modify-write.
 *
 * Alter @pc as required for unwinding.  Return the type of the
 * guest memory access -- host reads may be for guest execution.
 */
MMUAccessType adjust_signal_pc(uintptr_t *pc, bool is_write);

/**
 * handle_sigsegv_accerr_write:
 * @cpu: the cpu context
 * @old_set: the sigset_t from the signal ucontext_t
 * @host_pc: the host pc, adjusted for the signal
 * @host_addr: the host address of the fault
 *
 * Return true if the write fault has been handled, and should be re-tried.
 */
bool handle_sigsegv_accerr_write(CPUState *cpu, sigset_t *old_set,
                                 uintptr_t host_pc, abi_ptr guest_addr);

/**
 * cpu_loop_exit_sigsegv:
 * @cpu: the cpu context
 * @addr: the guest address of the fault
 * @access_type: access was read/write/execute
 * @maperr: true for invalid page, false for permission fault
 * @ra: host pc for unwinding
 *
 * Use the TCGCPUOps hook to record cpu state, do guest operating system
 * specific things to raise SIGSEGV, and jump to the main cpu loop.
 */
G_NORETURN void cpu_loop_exit_sigsegv(CPUState *cpu, target_ulong addr,
                                      MMUAccessType access_type,
                                      bool maperr, uintptr_t ra);

/**
 * cpu_loop_exit_sigbus:
 * @cpu: the cpu context
 * @addr: the guest address of the alignment fault
 * @access_type: access was read/write/execute
 * @ra: host pc for unwinding
 *
 * Use the TCGCPUOps hook to record cpu state, do guest operating system
 * specific things to raise SIGBUS, and jump to the main cpu loop.
 */
G_NORETURN void cpu_loop_exit_sigbus(CPUState *cpu, target_ulong addr,
                                     MMUAccessType access_type,
                                     uintptr_t ra);

#else
static inline void mmap_lock(void) {}
static inline void mmap_unlock(void) {}
#define WITH_MMAP_LOCK_GUARD()

void tlb_reset_dirty(CPUState *cpu, ram_addr_t start1, ram_addr_t length);
void tlb_set_dirty(CPUState *cpu, vaddr addr);

MemoryRegionSection *
address_space_translate_for_iotlb(CPUState *cpu, int asidx, hwaddr addr,
                                  hwaddr *xlat, hwaddr *plen,
                                  MemTxAttrs attrs, int *prot);
hwaddr memory_region_section_get_iotlb(CPUState *cpu,
                                       MemoryRegionSection *section);
#endif

#endif
