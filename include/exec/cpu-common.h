#ifndef CPU_COMMON_H
#define CPU_COMMON_H

/* CPU interfaces that are target independent.  */

#include "qemu/osdep.h"
#include "qemu/typedefs.h"
#ifndef CONFIG_USER_ONLY
#include "exec/hwaddr.h"
#endif
#include "exec/page-vary.h"
#include "hw/core/cpu.h"

#define EXCP_INTERRUPT  0x10000 /* async interruption */
#define EXCP_HLT        0x10001 /* hlt instruction reached */
#define EXCP_DEBUG      0x10002 /* cpu stopped after a breakpoint or singlestep */
#define EXCP_HALTED     0x10003 /* cpu is halted (waiting for external event) */
#define EXCP_YIELD      0x10004 /* cpu wants to yield timeslice to another */
#define EXCP_ATOMIC     0x10005 /* stop-the-world and emulate atomic */

// TODO(anjo): Merge with cpu.h?

/**
 * Variable page size macros
 *
 * TARGET_PAGE_BITS_VARY is assumed for softmmu targets so
 * these macros are target independent.  This is checked in
 * cpu-all.h.
 */
extern const TargetPageBits target_page;
#ifdef CONFIG_DEBUG_TCG
#define TARGET_PAGE_BITS_MIN ({ assert(target_page.decided); \
                                target_page.bits_min; })
#define TARGET_PAGE_BITS   ({ assert(target_page.decided); target_page.bits; })
#define TARGET_PAGE_MASK   ({ assert(target_page.decided); \
                              (int)target_page.mask; })
#else
#define TARGET_PAGE_BITS_MIN target_page.bits_min
#define TARGET_PAGE_BITS     target_page.bits
#define TARGET_PAGE_MASK     ((int)target_page.mask)
#endif
#define TARGET_PAGE_SIZE   (-(int)TARGET_PAGE_MASK)
#define TARGET_PAGE_ALIGN(addr) ROUND_UP((addr), TARGET_PAGE_SIZE)

#ifdef CONFIG_USER_ONLY

/*
 * Allow some level of source compatibility with softmmu.  We do not
 * support any of the more exotic features, so only invalid pages may
 * be signaled by probe_access_flags().
 */
#define TLB_INVALID_MASK    (1 << (TARGET_PAGE_BITS_MIN - 1))
#define TLB_MMIO            (1 << (TARGET_PAGE_BITS_MIN - 2))
#define TLB_WATCHPOINT      0

#else

/*
 * Flags stored in the low bits of the TLB virtual address.
 * These are defined so that fast path ram access is all zeros.
 * The flags all must be between TARGET_PAGE_BITS and
 * maximum address alignment bit.
 *
 * Use TARGET_PAGE_BITS_MIN so that these bits are constant
 * when TARGET_PAGE_BITS_VARY is in effect.
 *
 * The count, if not the placement of these bits is known
 * to tcg/tcg-op-ldst.c, check_max_alignment().
 */
/* Zero if TLB entry is valid.  */
#define TLB_INVALID_MASK    (1 << (TARGET_PAGE_BITS_MIN - 1))
/* Set if TLB entry references a clean RAM page.  The iotlb entry will
   contain the page physical address.  */
#define TLB_NOTDIRTY        (1 << (TARGET_PAGE_BITS_MIN - 2))
/* Set if TLB entry is an IO callback.  */
#define TLB_MMIO            (1 << (TARGET_PAGE_BITS_MIN - 3))
/* Set if TLB entry writes ignored.  */
#define TLB_DISCARD_WRITE   (1 << (TARGET_PAGE_BITS_MIN - 4))
/* Set if the slow path must be used; more flags in CPUTLBEntryFull. */
#define TLB_FORCE_SLOW      (1 << (TARGET_PAGE_BITS_MIN - 5))

/*
 * Use this mask to check interception with an alignment mask
 * in a TCG backend.
 */
#define TLB_FLAGS_MASK \
    (TLB_INVALID_MASK | TLB_NOTDIRTY | TLB_MMIO \
    | TLB_FORCE_SLOW | TLB_DISCARD_WRITE)

/*
 * Flags stored in CPUTLBEntryFull.slow_flags[x].
 * TLB_FORCE_SLOW must be set in CPUTLBEntry.addr_idx[x].
 */
/* Set if TLB entry requires byte swap.  */
#define TLB_BSWAP            (1 << 0)
/* Set if TLB entry contains a watchpoint.  */
#define TLB_WATCHPOINT       (1 << 1)

#define TLB_SLOW_FLAGS_MASK  (TLB_BSWAP | TLB_WATCHPOINT)

/* The two sets of flags must not overlap. */
// TODO(anjo): Move to some function
//QEMU_BUILD_BUG_ON(TLB_FLAGS_MASK & TLB_SLOW_FLAGS_MASK);

/**
 * tlb_hit_page: return true if page aligned @addr is a hit against the
 * TLB entry @tlb_addr
 *
 * @addr: virtual address to test (must be page aligned)
 * @tlb_addr: TLB entry address (a CPUTLBEntry addr_read/write/code value)
 */
static inline bool tlb_hit_page(uint64_t tlb_addr, vaddr addr)
{
    return addr == (tlb_addr & (TARGET_PAGE_MASK | TLB_INVALID_MASK));
}

/**
 * tlb_hit: return true if @addr is a hit against the TLB entry @tlb_addr
 *
 * @addr: virtual address to test (need not be page aligned)
 * @tlb_addr: TLB entry address (a CPUTLBEntry addr_read/write/code value)
 */
static inline bool tlb_hit(uint64_t tlb_addr, vaddr addr)
{
    return tlb_hit_page(tlb_addr, addr & TARGET_PAGE_MASK);
}

#endif /* !CONFIG_USER_ONLY */

void cpu_exec_init_all(void);
void cpu_exec_step_atomic(CPUState *cpu);

/* Using intptr_t ensures that qemu_*_page_mask is sign-extended even
 * when intptr_t is 32-bit and we are aligning a long long.
 */
extern uintptr_t qemu_host_page_size;
extern intptr_t qemu_host_page_mask;

#define HOST_PAGE_ALIGN(addr) ROUND_UP((addr), qemu_host_page_size)
#define REAL_HOST_PAGE_ALIGN(addr) ROUND_UP((addr), qemu_real_host_page_size())

/* The CPU list lock nests outside page_(un)lock or mmap_(un)lock */
extern QemuMutex qemu_cpu_list_lock;
void qemu_init_cpu_list(void);
void cpu_list_lock(void);
void cpu_list_unlock(void);
unsigned int cpu_list_generation_id_get(void);

void tcg_flush_softmmu_tlb(CPUState *cs);
void tcg_flush_jmp_cache(CPUState *cs);

void tcg_iommu_init_notifier_list(CPUState *cpu);
void tcg_iommu_free_notifier_list(CPUState *cpu);

#if !defined(CONFIG_USER_ONLY)

enum device_endian {
    DEVICE_NATIVE_ENDIAN,
    DEVICE_BIG_ENDIAN,
    DEVICE_LITTLE_ENDIAN,
};

#if HOST_BIG_ENDIAN
#define DEVICE_HOST_ENDIAN DEVICE_BIG_ENDIAN
#else
#define DEVICE_HOST_ENDIAN DEVICE_LITTLE_ENDIAN
#endif

/* address in the RAM (different from a physical address) */
#if defined(CONFIG_XEN_BACKEND)
typedef uint64_t ram_addr_t;
#  define RAM_ADDR_MAX UINT64_MAX
#  define RAM_ADDR_FMT "%" PRIx64
#else
typedef uintptr_t ram_addr_t;
#  define RAM_ADDR_MAX UINTPTR_MAX
#  define RAM_ADDR_FMT "%" PRIxPTR
#endif

/* memory API */

void qemu_ram_remap(ram_addr_t addr, ram_addr_t length);
/* This should not be used by devices.  */
ram_addr_t qemu_ram_addr_from_host(void *ptr);
ram_addr_t qemu_ram_addr_from_host_nofail(void *ptr);
RAMBlock *qemu_ram_block_by_name(const char *name);
RAMBlock *qemu_ram_block_from_host(void *ptr, bool round_offset,
                                   ram_addr_t *offset);
ram_addr_t qemu_ram_block_host_offset(RAMBlock *rb, void *host);
void qemu_ram_set_idstr(RAMBlock *block, const char *name, DeviceState *dev);
void qemu_ram_unset_idstr(RAMBlock *block);
const char *qemu_ram_get_idstr(RAMBlock *rb);
void *qemu_ram_get_host_addr(RAMBlock *rb);
ram_addr_t qemu_ram_get_offset(RAMBlock *rb);
ram_addr_t qemu_ram_get_used_length(RAMBlock *rb);
ram_addr_t qemu_ram_get_max_length(RAMBlock *rb);
bool qemu_ram_is_shared(RAMBlock *rb);
bool qemu_ram_is_noreserve(RAMBlock *rb);
bool qemu_ram_is_uf_zeroable(RAMBlock *rb);
void qemu_ram_set_uf_zeroable(RAMBlock *rb);
bool qemu_ram_is_migratable(RAMBlock *rb);
void qemu_ram_set_migratable(RAMBlock *rb);
void qemu_ram_unset_migratable(RAMBlock *rb);
bool qemu_ram_is_named_file(RAMBlock *rb);
int qemu_ram_get_fd(RAMBlock *rb);

size_t qemu_ram_pagesize(RAMBlock *block);
size_t qemu_ram_pagesize_largest(void);

/**
 * cpu_address_space_init:
 * @cpu: CPU to add this address space to
 * @asidx: integer index of this address space
 * @prefix: prefix to be used as name of address space
 * @mr: the root memory region of address space
 *
 * Add the specified address space to the CPU's cpu_ases list.
 * The address space added with @asidx 0 is the one used for the
 * convenience pointer cpu->as.
 * The target-specific code which registers ASes is responsible
 * for defining what semantics address space 0, 1, 2, etc have.
 *
 * Before the first call to this function, the caller must set
 * cpu->num_ases to the total number of address spaces it needs
 * to support.
 *
 * Note that with KVM only one address space is supported.
 */
void cpu_address_space_init(CPUState *cpu, int asidx,
                            const char *prefix, MemoryRegion *mr);

void cpu_physical_memory_rw(hwaddr addr, void *buf,
                            hwaddr len, bool is_write);
static inline void cpu_physical_memory_read(hwaddr addr,
                                            void *buf, hwaddr len)
{
    cpu_physical_memory_rw(addr, buf, len, false);
}
static inline void cpu_physical_memory_write(hwaddr addr,
                                             const void *buf, hwaddr len)
{
    cpu_physical_memory_rw(addr, (void *)buf, len, true);
}
void *cpu_physical_memory_map(hwaddr addr,
                              hwaddr *plen,
                              bool is_write);
void cpu_physical_memory_unmap(void *buffer, hwaddr len,
                               bool is_write, hwaddr access_len);
void cpu_register_map_client(QEMUBH *bh);
void cpu_unregister_map_client(QEMUBH *bh);

bool cpu_physical_memory_is_io(hwaddr phys_addr);

/* Coalesced MMIO regions are areas where write operations can be reordered.
 * This usually implies that write operations are side-effect free.  This allows
 * batching which can make a major impact on performance when using
 * virtualization.
 */
void qemu_flush_coalesced_mmio_buffer(void);

void cpu_flush_icache_range(hwaddr start, hwaddr len);

typedef int (RAMBlockIterFunc)(RAMBlock *rb, void *opaque);

int qemu_ram_foreach_block(RAMBlockIterFunc func, void *opaque);
int ram_block_discard_range(RAMBlock *rb, uint64_t start, size_t length);

#endif

/* Returns: 0 on success, -1 on error */
int cpu_memory_rw_debug(CPUState *cpu, vaddr addr,
                        void *ptr, size_t len, bool is_write);

/* vl.c */
void list_cpus(void);

#ifdef CONFIG_TCG
/**
 * cpu_unwind_state_data:
 * @cpu: the cpu context
 * @host_pc: the host pc within the translation
 * @data: output data
 *
 * Attempt to load the the unwind state for a host pc occurring in
 * translated code.  If @host_pc is not in translated code, the
 * function returns false; otherwise @data is loaded.
 * This is the same unwind info as given to restore_state_to_opc.
 */
bool cpu_unwind_state_data(CPUState *cpu, uintptr_t host_pc, uint64_t *data);

/**
 * cpu_restore_state:
 * @cpu: the cpu context
 * @host_pc: the host pc within the translation
 * @return: true if state was restored, false otherwise
 *
 * Attempt to restore the state for a fault occurring in translated
 * code. If @host_pc is not in translated code no state is
 * restored and the function returns false.
 */
bool cpu_restore_state(CPUState *cpu, uintptr_t host_pc);

G_NORETURN void cpu_loop_exit_noexc(CPUState *cpu);
G_NORETURN void cpu_loop_exit_atomic(CPUState *cpu, uintptr_t pc);
#endif /* CONFIG_TCG */
G_NORETURN void cpu_loop_exit(CPUState *cpu);
G_NORETURN void cpu_loop_exit_restore(CPUState *cpu, uintptr_t pc);

/* current cflags for hashing/comparison */
uint32_t curr_cflags(CPUState *cpu);

/* same as PROT_xxx */
#define PAGE_READ      0x0001
#define PAGE_WRITE     0x0002
#define PAGE_EXEC      0x0004
#define PAGE_BITS      (PAGE_READ | PAGE_WRITE | PAGE_EXEC)
#define PAGE_VALID     0x0008
/*
 * Original state of the write flag (used when tracking self-modifying code)
 */
#define PAGE_WRITE_ORG 0x0010
/*
 * Invalidate the TLB entry immediately, helpful for s390x
 * Low-Address-Protection. Used with PAGE_WRITE in tlb_set_page_with_attrs()
 */
#define PAGE_WRITE_INV 0x0020
/* For use with page_set_flags: page is being replaced; target_data cleared. */
#define PAGE_RESET     0x0040
/* For linux-user, indicates that the page is MAP_ANON. */
#define PAGE_ANON      0x0080

/* Target-specific bits that will be used via page_get_flags().  */
#define PAGE_TARGET_1  0x0200
#define PAGE_TARGET_2  0x0400

/*
 * For linux-user, indicates that the page is mapped with the same semantics
 * in both guest and host.
 */
#define PAGE_PASSTHROUGH 0x0800

/* Flags for use in ENV->INTERRUPT_PENDING.

   The numbers assigned here are non-sequential in order to preserve
   binary compatibility with the vmstate dump.  Bit 0 (0x0001) was
   previously used for CPU_INTERRUPT_EXIT, and is cleared when loading
   the vmstate dump.  */

/* External hardware interrupt pending.  This is typically used for
   interrupts from devices.  */
#define CPU_INTERRUPT_HARD        0x0002

/* Exit the current TB.  This is typically used when some system-level device
   makes some change to the memory mapping.  E.g. the a20 line change.  */
#define CPU_INTERRUPT_EXITTB      0x0004

/* Halt the CPU.  */
#define CPU_INTERRUPT_HALT        0x0020

/* Debug event pending.  */
#define CPU_INTERRUPT_DEBUG       0x0080

/* Reset signal.  */
#define CPU_INTERRUPT_RESET       0x0400

/* Several target-specific external hardware interrupts.  Each target/cpu.h
   should define proper names based on these defines.  */
#define CPU_INTERRUPT_TGT_EXT_0   0x0008
#define CPU_INTERRUPT_TGT_EXT_1   0x0010
#define CPU_INTERRUPT_TGT_EXT_2   0x0040
#define CPU_INTERRUPT_TGT_EXT_3   0x0200
#define CPU_INTERRUPT_TGT_EXT_4   0x1000

/* Several target-specific internal interrupts.  These differ from the
   preceding target-specific interrupts in that they are intended to
   originate from within the cpu itself, typically in response to some
   instruction being executed.  These, therefore, are not masked while
   single-stepping within the debugger.  */
#define CPU_INTERRUPT_TGT_INT_0   0x0100
#define CPU_INTERRUPT_TGT_INT_1   0x0800
#define CPU_INTERRUPT_TGT_INT_2   0x2000

/* First unused bit: 0x4000.  */

/* The set of all bits that should be masked when single-stepping.  */
#define CPU_INTERRUPT_SSTEP_MASK \
    (CPU_INTERRUPT_HARD          \
     | CPU_INTERRUPT_TGT_EXT_0   \
     | CPU_INTERRUPT_TGT_EXT_1   \
     | CPU_INTERRUPT_TGT_EXT_2   \
     | CPU_INTERRUPT_TGT_EXT_3   \
     | CPU_INTERRUPT_TGT_EXT_4)

/* accel/tcg/cpu-exec.c */
int cpu_exec(CPUState *cpu);

/**
 * env_archcpu(env)
 * @env: The architecture environment
 *
 * Return the ArchCPU associated with the environment.
 */
static inline ArchCPU *env_archcpu(CPUArchState *env)
{
    return (void *)env - sizeof(CPUState);
}

/**
 * env_cpu(env)
 * @env: The architecture environment
 *
 * Return the CPUState associated with the environment.
 */
static inline CPUState *env_cpu(CPUArchState *env)
{
    return (void *)env - sizeof(CPUState);
}

#endif /* CPU_COMMON_H */
