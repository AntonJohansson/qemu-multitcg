#include "qemu/osdep.h"
#include "exec/cpu_ldst.h"
#include "exec/memop.h"
#include "exec/memopidx.h"
#include "exec/exec-common.h"

uint32_t cpu_ldub_code(CPUArchState *env, abi_ptr addr)
{
    MemOpIdx oi = make_memop_idx(MO_UB, cpu_mmu_index(env, true));
    return do_ld1_mmu(env_cpu(env), addr, oi, 0, MMU_INST_FETCH);
}

uint32_t cpu_lduw_code(CPUArchState *env, abi_ptr addr)
{
    MemOpIdx oi = make_memop_idx(MO_TEUW, cpu_mmu_index(env, true));
    return do_ld2_mmu(env_cpu(env), addr, oi, 0, MMU_INST_FETCH);
}

uint32_t cpu_ldl_code(CPUArchState *env, abi_ptr addr)
{
    MemOpIdx oi = make_memop_idx(MO_TEUL, cpu_mmu_index(env, true));
    return do_ld4_mmu(env_cpu(env), addr, oi, 0, MMU_INST_FETCH);
}

uint64_t cpu_ldq_code(CPUArchState *env, abi_ptr addr)
{
    MemOpIdx oi = make_memop_idx(MO_TEUQ, cpu_mmu_index(env, true));
    return do_ld8_mmu(env_cpu(env), addr, oi, 0, MMU_INST_FETCH);
}

uint32_t cpu_ldub_data_ra(CPUArchState *env, abi_ptr addr, uintptr_t ra)
{
    return cpu_ldub_mmuidx_ra(env, addr, cpu_mmu_index(env, false), ra);
}

uint32_t cpu_lduw_be_data_ra(CPUArchState *env, abi_ptr addr, uintptr_t ra)
{
    return cpu_lduw_be_mmuidx_ra(env, addr, cpu_mmu_index(env, false), ra);
}

uint32_t cpu_ldl_be_data_ra(CPUArchState *env, abi_ptr addr, uintptr_t ra)
{
    return cpu_ldl_be_mmuidx_ra(env, addr, cpu_mmu_index(env, false), ra);
}

uint64_t cpu_ldq_be_data_ra(CPUArchState *env, abi_ptr addr, uintptr_t ra)
{
    return cpu_ldq_be_mmuidx_ra(env, addr, cpu_mmu_index(env, false), ra);
}

uint32_t cpu_lduw_le_data_ra(CPUArchState *env, abi_ptr addr, uintptr_t ra)
{
    return cpu_lduw_le_mmuidx_ra(env, addr, cpu_mmu_index(env, false), ra);
}

uint32_t cpu_ldl_le_data_ra(CPUArchState *env, abi_ptr addr, uintptr_t ra)
{
    return cpu_ldl_le_mmuidx_ra(env, addr, cpu_mmu_index(env, false), ra);
}

uint64_t cpu_ldq_le_data_ra(CPUArchState *env, abi_ptr addr, uintptr_t ra)
{
    return cpu_ldq_le_mmuidx_ra(env, addr, cpu_mmu_index(env, false), ra);
}

void cpu_stb_data_ra(CPUArchState *env, abi_ptr addr,
                     uint32_t val, uintptr_t ra)
{
    cpu_stb_mmuidx_ra(env, addr, val, cpu_mmu_index(env, false), ra);
}

void cpu_stw_be_data_ra(CPUArchState *env, abi_ptr addr,
                        uint32_t val, uintptr_t ra)
{
    cpu_stw_be_mmuidx_ra(env, addr, val, cpu_mmu_index(env, false), ra);
}

void cpu_stl_be_data_ra(CPUArchState *env, abi_ptr addr,
                        uint32_t val, uintptr_t ra)
{
    cpu_stl_be_mmuidx_ra(env, addr, val, cpu_mmu_index(env, false), ra);
}

void cpu_stq_be_data_ra(CPUArchState *env, abi_ptr addr,
                        uint64_t val, uintptr_t ra)
{
    cpu_stq_be_mmuidx_ra(env, addr, val, cpu_mmu_index(env, false), ra);
}

void cpu_stw_le_data_ra(CPUArchState *env, abi_ptr addr,
                        uint32_t val, uintptr_t ra)
{
    cpu_stw_le_mmuidx_ra(env, addr, val, cpu_mmu_index(env, false), ra);
}

void cpu_stl_le_data_ra(CPUArchState *env, abi_ptr addr,
                        uint32_t val, uintptr_t ra)
{
    cpu_stl_le_mmuidx_ra(env, addr, val, cpu_mmu_index(env, false), ra);
}

void cpu_stq_le_data_ra(CPUArchState *env, abi_ptr addr,
                        uint64_t val, uintptr_t ra)
{
    cpu_stq_le_mmuidx_ra(env, addr, val, cpu_mmu_index(env, false), ra);
}
