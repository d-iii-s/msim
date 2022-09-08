#include <stdint.h>
#include <sys/time.h>
#include "csr.h"
#include "cpu.h"
#include "../../../assert.h"

#define RV_A_EXTENSION_BITS  UINT32_C(1<<0)
#define RV_C_EXTENSION_BITS  UINT32_C(1<<2)
#define RV_D_EXTENSION_BITS  UINT32_C(1<<3)
#define RV_E_EXTENSION_BITS  UINT32_C(1<<4)
#define RV_F_EXTENSION_BITS  UINT32_C(1<<5)
#define RV_H_EXTENSION_BITS  UINT32_C(1<<7)
#define RV_I_EXTENSION_BITS  UINT32_C(1<<8)
#define RV_M_EXTENSION_BITS  UINT32_C(1<<12)
#define RV_Q_EXTENSION_BITS  UINT32_C(1<<16)
#define RV_S_IMPLEMENTED_BITS  UINT32_C(1<<18)
#define RV_U_IMPLEMENTED_BITS  UINT32_C(1<<20)

#define RV_32_MXLEN_BITS UINT32_C(0x40000000)
#define RV_64_MXLEN_BITS UINT32_C(0x80000000)
#define RV_128_MXLEN_BITS UINT32_C(0xC0000000)

#define RV_ISA  RV_32_MXLEN_BITS | RV_I_EXTENSION_BITS | RV_M_EXTENSION_BITS | RV_A_EXTENSION_BITS | RV_U_IMPLEMENTED_BITS | RV_S_IMPLEMENTED_BITS


#define RV_VENDOR_ID 0
#define RV_ARCH_ID 0
#define RV_IMPLEMENTATION_ID 0

void rv_init_csr(csr_t *csr, unsigned int procno){
    
    csr->misa = RV_ISA;
    csr->mvendorid = RV_VENDOR_ID;
    csr->marchid = RV_ARCH_ID;
    csr->mimpid = RV_IMPLEMENTATION_ID;
    csr->mhartid = procno;

    //TODO: rest
}

#define minimal_privilege(priv, cpu) {if(cpu->priv_mode < priv) return rv_exc_illegal_instruction;}
#define default_csr_functions(csr_name, priv)                                       \
    static rv_exc_t csr_name##_read(rv_cpu_t* cpu, int csr, uint32_t* target){      \
        minimal_privilege(priv, cpu);                                               \
        *target = cpu->csr.csr_name;                                                \
        return rv_exc_none;                                                         \
    }                                                                               \
    static rv_exc_t csr_name##_write(rv_cpu_t* cpu, int csr, uint32_t target){      \
        minimal_privilege(priv, cpu);                                               \
        cpu->csr.csr_name = target;                                                 \
        return rv_exc_none;                                                         \
    }                                                                               \
    static rv_exc_t csr_name##_set(rv_cpu_t* cpu, int csr, uint32_t target){        \
        minimal_privilege(priv, cpu);                                               \
        cpu->csr.csr_name |= target;                                                \
        return rv_exc_none;                                                         \
    }                                                                               \
    static rv_exc_t csr_name##_clear(rv_cpu_t* cpu, int csr, uint32_t target){      \
        minimal_privilege(priv, cpu);                                               \
        cpu->csr.csr_name &= ~target;                                               \
        return rv_exc_none;                                                         \
    }

static rv_exc_t invalid_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}
static rv_exc_t invalid_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_illegal_instruction;
}

// TODO: mtime & mtimecmp

#define is_counter_enabled_m(cpu, counter) (cpu->csr.mcounteren & (1<<counter))
#define is_counter_enabled_s(cpu, counter) (cpu->csr.scounteren & (1<<counter))
#define is_high_counter(csr) (csr & 0x080)

static uint64_t current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    uint64_t milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    return milliseconds;
}

static inline void read_counter_csr_unchecked(rv_cpu_t* cpu, int csr, uint32_t* target){
    int counter = csr & 0x1F;

    switch(counter){
        case (csr_cycle & 0x1F): {
            *target = (uint32_t)(is_high_counter(csr) ? cpu->csr.cycle >> 32 : cpu->csr.cycle); 
            break;
        }
        case (csr_time & 0x1F): {
            uint64_t time = current_timestamp();
            *target = (uint32_t)(is_high_counter(csr) ? time >> 32 : time);
            break;
        }
        case (csr_instret & 0x1F): {
            *target = (uint32_t)(is_high_counter(csr) ? cpu->csr.instret >> 32 : cpu->csr.instret); 
            break;
        }
        default: {
            uint64_t hpc = cpu->csr.hpmcounters[counter - 3];
            *target = (uint32_t)(is_high_counter(csr) ? hpc >> 32 : hpc);
            break;
        }
    }
}

static rv_exc_t counter_read(rv_cpu_t* cpu, int csr, uint32_t* target){

    // lowest 5 bits
    int counter = csr * 0x1F;

    if(rv_csr_min_priv_mode(csr) != rv_mmode){

        if(cpu->priv_mode == rv_smode && !is_counter_enabled_m(cpu, counter))
            return rv_exc_illegal_instruction;
        
        if(cpu->priv_mode == rv_umode && !(is_counter_enabled_m(cpu, counter) && is_counter_enabled_s(cpu, counter)))
            return rv_exc_illegal_instruction;

    }
    else if (cpu->priv_mode != rv_mmode){
        return rv_exc_illegal_instruction;
    }
    else if (counter == (csr_time & 0x1F)){
        // mtime is not a csr
        return rv_exc_illegal_instruction;
    }

    read_counter_csr_unchecked(cpu, csr, target);

    return rv_exc_none;
}

static rv_exc_t counter_write(rv_cpu_t* cpu, int csr, uint32_t target){

    // only mmode can write to counters
    minimal_privilege(rv_mmode, cpu);

    // global counters are r/o
    if(rv_csr_min_priv_mode(csr) != rv_mmode) return rv_exc_illegal_instruction;

    int counter = csr & 0x1F;

    // mtime is not a csr
    if (counter == (csr_time & 0x1F)) return rv_exc_illegal_instruction;

    uint64_t val = 0;
    uint64_t mask = 0;

    if(is_high_counter(csr)){
        val = ((uint64_t)target) << 32;
        mask = 0x00000000FFFFFFFF;
    }
    else {
        val = target;
        mask = 0xFFFFFFFF00000000;
    }

    switch(csr){
        case (csr_mcycle): {
            cpu->csr.cycle = (cpu->csr.cycle & mask) | val;
            break;
        }
        case (csr_minstret): {
            cpu->csr.instret = (cpu->csr.instret & mask) | val;
            break;
        }
        default: {
            uint64_t hpc = cpu->csr.hpmcounters[counter - 3];
            cpu->csr.hpmcounters[counter - 3] = (hpc & mask) | val;
            break;
        }
    }

    return rv_exc_none;
}

static rv_exc_t counter_set(rv_cpu_t* cpu, int csr, uint32_t target){

    // only mmode can write to counters
    minimal_privilege(rv_mmode, cpu);

    // global counters are r/o
    if(rv_csr_min_priv_mode(csr) != rv_mmode) return rv_exc_illegal_instruction;

    int counter = csr & 0x1F;

    // mtime is not a csr
    if (counter == (csr_time & 0x1F)) return rv_exc_illegal_instruction;

    uint64_t val = is_high_counter(csr) ? ((uint64_t)target) << 32 : target;

    switch(csr){
        case (csr_mcycle): {
            cpu->csr.cycle |= val;
            break;
        }
        case (csr_minstret): {
            cpu->csr.instret |= val;
            break;
        }
        default: {
            cpu->csr.hpmcounters[counter - 3] |= val;
            break;
        }
    }

    return rv_exc_none;
}

static rv_exc_t counter_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    // only mmode can write to counters
    minimal_privilege(rv_mmode, cpu);

    // global counters are r/o
    if(rv_csr_min_priv_mode(csr) != rv_mmode) return rv_exc_illegal_instruction;

    int counter = csr & 0x1F;

    // mtime is not a csr
    if (counter == (csr_time & 0x1F)) return rv_exc_illegal_instruction;

    uint64_t val = is_high_counter(csr) ? ((uint64_t)target) << 32 : target;

    switch(csr){
        case (csr_mcycle): {
            cpu->csr.cycle &= ~val;
            break;
        }
        case (csr_minstret): {
            cpu->csr.instret &= ~val;
            break;
        }
        default: {
            cpu->csr.hpmcounters[counter - 3] &= ~val;
            break;
        }
    }

    return rv_exc_none;
}

static rv_exc_t mcountinhibit_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mcountinhibit;
    return rv_exc_none;
}

static rv_exc_t mcountinhibit_write(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mcountinhibit = target;
    return rv_exc_none;
}

static rv_exc_t mcountinhibit_set(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mcountinhibit |= target;
    return rv_exc_none;
}

static rv_exc_t mcountinhibit_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_mmode, cpu);
    cpu->csr.mcountinhibit &= ~target;
    return rv_exc_none;
}

static rv_exc_t mhmpevent_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mhmpevent_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mhmpevent_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mhmpevent_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t pmpcfg_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t pmpcfg_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t pmpcfg_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t pmpcfg_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t pmpaddr_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t pmpaddr_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t pmpaddr_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t pmpaddr_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t sstatus_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t sstatus_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t sstatus_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t sstatus_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

#define sei_mask (1U << 9)
#define sti_mask (1U << 5)
#define ssi_mask (1U << 1)
#define si_mask (sei_mask | sti_mask | ssi_mask)

static rv_exc_t sie_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.mie & si_mask;
    return rv_exc_none;
}

static rv_exc_t sie_write(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    
    // write only to si bits, preserve rest
    cpu->csr.mie &= ~si_mask;
    cpu->csr.mie |= target & si_mask;
    return rv_exc_none;
}

static rv_exc_t sie_set(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.mie |= target & si_mask;
    return rv_exc_none;
}

static rv_exc_t sie_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.mie &= ~(target & si_mask);
    return rv_exc_none;
}

static rv_exc_t sip_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.mip & si_mask;
    return rv_exc_none;
}

static rv_exc_t sip_write(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    
    // only ssip is writable
    cpu->csr.mip &= ~ssi_mask;
    cpu->csr.mip |= target & ssi_mask;
    return rv_exc_none;
}

static rv_exc_t sip_set(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.mip |= target & ssi_mask;
    return rv_exc_none;
}

static rv_exc_t sip_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.mip &= ~(target & ssi_mask);
    return rv_exc_none;
}

#define scvec_mask 0xFFFFFFFD

static rv_exc_t stvec_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.stvec;
    return rv_exc_none;
}

static rv_exc_t stvec_write(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.stvec = target & scvec_mask;
    return rv_exc_none;
}

static rv_exc_t stvec_set(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.stvec |= target & scvec_mask;
    return rv_exc_none;
}

static rv_exc_t stvec_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.stvec &= ~(target & scvec_mask);
    return rv_exc_none;
}

default_csr_functions(scounteren, rv_smode)

#define senvcfg_mask 0x71

static rv_exc_t senvcfg_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.senvcfg;
    return rv_exc_none;
}

static rv_exc_t senvcfg_set(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    // Setting WPRI bits causes exception
    if((target & ~senvcfg_mask) != 0) return rv_exc_illegal_instruction;

    cpu->csr.senvcfg |= target & senvcfg_mask;
    return rv_exc_none;
}

static rv_exc_t senvcfg_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    // Clearing WPRI bits causes exception
    if((target & ~senvcfg_mask) != 0) return rv_exc_illegal_instruction;

    cpu->csr.senvcfg &= ~(target & senvcfg_mask);
    return rv_exc_none;
}

default_csr_functions(sscratch, rv_smode)

#define sepc_mask 0xFFFFFFFC

static rv_exc_t sepc_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.sepc;
    return rv_exc_none;
}

static rv_exc_t sepc_write(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.sepc = target & sepc_mask;
    return rv_exc_none;
}

static rv_exc_t sepc_set(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.sepc |= target & sepc_mask;
    return rv_exc_none;
}

static rv_exc_t sepc_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);
    cpu->csr.sepc &= ~(target & sepc_mask);
    return rv_exc_none;
}

static bool is_exception(uint32_t num){
    switch(num){
        case (rv_exc_supervisor_software_interrupt):
        case (rv_exc_machine_software_interrupt):
        case (rv_exc_supervisor_timer_interrupt):
        case (rv_exc_machine_timer_interrupt):
        case (rv_exc_supervisor_external_interrupt):
        case (rv_exc_machine_external_interrupt):
        case (rv_exc_instruction_address_misaligned):
        case (rv_exc_instruction_access_fault):
        case (rv_exc_illegal_instruction):
        case (rv_exc_breakpoint):
        case (rv_exc_load_address_misaligned):
        case (rv_exc_load_access_fault):
        case (rv_exc_store_amo_address_misaligned):
        case (rv_exc_store_amo_access_fault):
        case (rv_exc_umode_environment_call):
        case (rv_exc_smode_environment_call):
        case (rv_exc_mmode_environment_call):
        case (rv_exc_instruction_page_fault):
        case (rv_exc_load_page_fault):
        case (rv_exc_store_amo_page_fault):
            return true;
        default:
            return false;
    }
}


static rv_exc_t scause_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    minimal_privilege(rv_smode, cpu);
    *target = cpu->csr.scause;
    return rv_exc_none;
}

static rv_exc_t scause_write(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);

    if(is_exception(target)){
        cpu->csr.scause = target;
    }
    else {
        return rv_exc_illegal_instruction;
    }

    return rv_exc_none;
}

static rv_exc_t scause_set(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);

    uint32_t val = target | cpu->csr.scause;
    if(is_exception(val)){
        cpu->csr.scause = val;
    }
    else {
        return rv_exc_illegal_instruction;
    }

    return rv_exc_none;
}

static rv_exc_t scause_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    minimal_privilege(rv_smode, cpu);

    uint32_t val = ~target & cpu->csr.scause;
    if(is_exception(val)){
        cpu->csr.scause = val;
    }
    else {
        return rv_exc_illegal_instruction;
    }
    
    return rv_exc_none;
}

default_csr_functions(stval, rv_smode)

static rv_exc_t satp_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t satp_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t satp_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t satp_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t scontext_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t scontext_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t scontext_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t scontext_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mvendorid_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mvendorid;
    return rv_exc_none;
}

static rv_exc_t marchid_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.marchid;
    return rv_exc_none;
}

static rv_exc_t mimpid_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mimpid;
    return rv_exc_none;
}

static rv_exc_t mhartid_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    minimal_privilege(rv_mmode, cpu);
    *target = cpu->csr.mhartid;
    return rv_exc_none;
}

static rv_exc_t mconfigptr_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mconfigptr_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mconfigptr_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mconfigptr_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mstatus_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mstatus_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mstatus_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mstatus_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t misa_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t misa_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t misa_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t misa_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t medeleg_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t medeleg_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t medeleg_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t medeleg_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mideleg_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mideleg_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mideleg_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mideleg_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mie_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mie_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mie_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mie_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtvec_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mtvec_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtvec_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtvec_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mcounteren_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mcounteren_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mcounteren_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mcounteren_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mstatush_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mstatush_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mstatush_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mstatush_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mscratch_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mscratch_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mscratch_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mscratch_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mepc_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mepc_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mepc_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mepc_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mcause_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mcause_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mcause_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mcause_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtval_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mtval_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtval_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtval_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mip_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mip_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mip_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mip_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtinst_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mtinst_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtinst_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtinst_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtval2_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mtval2_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtval2_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mtval2_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t menvcfg_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t menvcfg_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t menvcfg_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t menvcfg_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mevncfgh_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mevncfgh_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mevncfgh_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mevncfgh_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mseccfg_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mseccfg_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mseccfg_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mseccfg_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mseccfgh_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mseccfgh_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mseccfgh_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mseccfgh_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tselect_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t tselect_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tselect_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tselect_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tdata1_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t tdata1_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tdata1_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tdata1_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tdata2_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t tdata2_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tdata2_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tdata2_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tdata3_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t tdata3_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tdata3_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t tdata3_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mcontext_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t mcontext_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mcontext_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t mcontext_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dcsr_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t dcsr_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dcsr_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dcsr_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dpc_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t dpc_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dpc_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dpc_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dscratch0_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t dscratch0_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dscratch0_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dscratch0_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dscratch1_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_none;
}

static rv_exc_t dscratch1_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dscratch1_set(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

static rv_exc_t dscratch1_clear(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_none;
}

typedef rv_exc_t (*csr_read_func_t)(rv_cpu_t*, int csr, uint32_t*);
typedef rv_exc_t (*csr_write_func_t)(rv_cpu_t*, int csr, uint32_t);
typedef rv_exc_t (*csr_set_func_t)(rv_cpu_t*, int csr, uint32_t);
typedef rv_exc_t (*csr_clear_func_t)(rv_cpu_t*, int csr, uint32_t);

typedef struct {
    csr_read_func_t read;
    csr_write_func_t write;
    csr_set_func_t set;
    csr_clear_func_t clear;
} csr_ops_t;

static csr_ops_t get_csr_ops(int csr){
    csr_ops_t ops = {
        .read = invalid_read,
        .write = invalid_write,
        .set = invalid_write,
        .clear = invalid_write
    };

    switch(csr){
        case(csr_cycle):
        case(csr_time):
        case(csr_instret):
        case(csr_hpmcounter3):
        case(csr_hpmcounter4):
        case(csr_hpmcounter5):
        case(csr_hpmcounter6):
        case(csr_hpmcounter7):
        case(csr_hpmcounter8):
        case(csr_hpmcounter9):
        case(csr_hpmcounter10):
        case(csr_hpmcounter11):
        case(csr_hpmcounter12):
        case(csr_hpmcounter13):
        case(csr_hpmcounter14):
        case(csr_hpmcounter15):
        case(csr_hpmcounter16):
        case(csr_hpmcounter17):
        case(csr_hpmcounter18):
        case(csr_hpmcounter19):
        case(csr_hpmcounter20):
        case(csr_hpmcounter21):
        case(csr_hpmcounter22):
        case(csr_hpmcounter23):
        case(csr_hpmcounter24):
        case(csr_hpmcounter25):
        case(csr_hpmcounter26):
        case(csr_hpmcounter27):
        case(csr_hpmcounter28):
        case(csr_hpmcounter29):
        case(csr_hpmcounter30):
        case(csr_hpmcounter31):
        case(csr_cycleh):
        case(csr_timeh):
        case(csr_instreth):
        case(csr_hpmcounter3h):
        case(csr_hpmcounter4h):
        case(csr_hpmcounter5h):
        case(csr_hpmcounter6h):
        case(csr_hpmcounter7h):
        case(csr_hpmcounter8h):
        case(csr_hpmcounter9h):
        case(csr_hpmcounter10h):
        case(csr_hpmcounter11h):
        case(csr_hpmcounter12h):
        case(csr_hpmcounter13h):
        case(csr_hpmcounter14h):
        case(csr_hpmcounter15h):
        case(csr_hpmcounter16h):
        case(csr_hpmcounter17h):
        case(csr_hpmcounter18h):
        case(csr_hpmcounter19h):
        case(csr_hpmcounter20h):
        case(csr_hpmcounter21h):
        case(csr_hpmcounter22h):
        case(csr_hpmcounter23h):
        case(csr_hpmcounter24h):
        case(csr_hpmcounter25h):
        case(csr_hpmcounter26h):
        case(csr_hpmcounter27h):
        case(csr_hpmcounter28h):
        case(csr_hpmcounter29h):
        case(csr_hpmcounter30h):
        case(csr_hpmcounter31h):
        case(csr_mcycle):
        case(csr_minstret):
        case(csr_mhpmcounter3):
        case(csr_mhpmcounter4):
        case(csr_mhpmcounter5):
        case(csr_mhpmcounter6):
        case(csr_mhpmcounter7):
        case(csr_mhpmcounter8):
        case(csr_mhpmcounter9):
        case(csr_mhpmcounter10):
        case(csr_mhpmcounter11):
        case(csr_mhpmcounter12):
        case(csr_mhpmcounter13):
        case(csr_mhpmcounter14):
        case(csr_mhpmcounter15):
        case(csr_mhpmcounter16):
        case(csr_mhpmcounter17):
        case(csr_mhpmcounter18):
        case(csr_mhpmcounter19):
        case(csr_mhpmcounter20):
        case(csr_mhpmcounter21):
        case(csr_mhpmcounter22):
        case(csr_mhpmcounter23):
        case(csr_mhpmcounter24):
        case(csr_mhpmcounter25):
        case(csr_mhpmcounter26):
        case(csr_mhpmcounter27):
        case(csr_mhpmcounter28):
        case(csr_mhpmcounter29):
        case(csr_mhpmcounter30):
        case(csr_mhpmcounter31):
        case(csr_mcycleh):
        case(csr_minstreth):
        case(csr_mhpmcounter3h):
        case(csr_mhpmcounter4h):
        case(csr_mhpmcounter5h):
        case(csr_mhpmcounter6h):
        case(csr_mhpmcounter7h):
        case(csr_mhpmcounter8h):
        case(csr_mhpmcounter9h):
        case(csr_mhpmcounter10h):
        case(csr_mhpmcounter11h):
        case(csr_mhpmcounter12h):
        case(csr_mhpmcounter13h):
        case(csr_mhpmcounter14h):
        case(csr_mhpmcounter15h):
        case(csr_mhpmcounter16h):
        case(csr_mhpmcounter17h):
        case(csr_mhpmcounter18h):
        case(csr_mhpmcounter19h):
        case(csr_mhpmcounter20h):
        case(csr_mhpmcounter21h):
        case(csr_mhpmcounter22h):
        case(csr_mhpmcounter23h):
        case(csr_mhpmcounter24h):
        case(csr_mhpmcounter25h):
        case(csr_mhpmcounter26h):
        case(csr_mhpmcounter27h):
        case(csr_mhpmcounter28h):
        case(csr_mhpmcounter29h):
        case(csr_mhpmcounter30h):
        case(csr_mhpmcounter31h): {
            ops.read = counter_read;
            ops.write = counter_write;
            ops.set = counter_set;
            ops.clear = counter_clear;
            break;
        }

        case(csr_mhmpevent3):
        case(csr_mhmpevent4):
        case(csr_mhmpevent5):
        case(csr_mhmpevent6):
        case(csr_mhmpevent7):
        case(csr_mhmpevent8):
        case(csr_mhmpevent9):
        case(csr_mhmpevent10):
        case(csr_mhmpevent11):
        case(csr_mhmpevent12):
        case(csr_mhmpevent13):
        case(csr_mhmpevent14):
        case(csr_mhmpevent15):
        case(csr_mhmpevent16):
        case(csr_mhmpevent17):
        case(csr_mhmpevent18):
        case(csr_mhmpevent19):
        case(csr_mhmpevent20):
        case(csr_mhmpevent21):
        case(csr_mhmpevent22):
        case(csr_mhmpevent23):
        case(csr_mhmpevent24):
        case(csr_mhmpevent25):
        case(csr_mhmpevent26):
        case(csr_mhmpevent27):
        case(csr_mhmpevent28):
        case(csr_mhmpevent29):
        case(csr_mhmpevent30):
        case(csr_mhmpevent31): {
            ops.read = mhmpevent_read;
            ops.write = mhmpevent_write;
            ops.set = mhmpevent_set;
            ops.clear = mhmpevent_clear;
            break;
        }

        case(csr_pmpcfg0):
	    case(csr_pmpcfg1):
	    case(csr_pmpcfg2):
	    case(csr_pmpcfg3):
	    case(csr_pmpcfg4):
	    case(csr_pmpcfg5):
	    case(csr_pmpcfg6):
	    case(csr_pmpcfg7):
	    case(csr_pmpcfg8):
	    case(csr_pmpcfg9):
	    case(csr_pmpcfg10):
	    case(csr_pmpcfg11):
	    case(csr_pmpcfg12):
	    case(csr_pmpcfg13):
	    case(csr_pmpcfg14):
	    case(csr_pmpcfg15): {
            ops.read = pmpcfg_read;
            ops.write = pmpcfg_write;
            ops.set = pmpcfg_set;
            ops.clear = pmpcfg_clear;
            break;
        }

        case(csr_pmpaddr0):
        case(csr_pmpaddr1):
        case(csr_pmpaddr2):
        case(csr_pmpaddr3):
        case(csr_pmpaddr4):
        case(csr_pmpaddr5):
        case(csr_pmpaddr6):
        case(csr_pmpaddr7):
        case(csr_pmpaddr8):
        case(csr_pmpaddr9):
        case(csr_pmpaddr10):
        case(csr_pmpaddr11):
        case(csr_pmpaddr12):
        case(csr_pmpaddr13):
        case(csr_pmpaddr14):
        case(csr_pmpaddr15):
        case(csr_pmpaddr16):
        case(csr_pmpaddr17):
        case(csr_pmpaddr18):
        case(csr_pmpaddr19):
        case(csr_pmpaddr20):
        case(csr_pmpaddr21):
        case(csr_pmpaddr22):
        case(csr_pmpaddr23):
        case(csr_pmpaddr24):
        case(csr_pmpaddr25):
        case(csr_pmpaddr26):
        case(csr_pmpaddr27):
        case(csr_pmpaddr28):
        case(csr_pmpaddr29):
        case(csr_pmpaddr30):
        case(csr_pmpaddr31):
        case(csr_pmpaddr32):
        case(csr_pmpaddr33):
        case(csr_pmpaddr34):
        case(csr_pmpaddr35):
        case(csr_pmpaddr36):
        case(csr_pmpaddr37):
        case(csr_pmpaddr38):
        case(csr_pmpaddr39):
        case(csr_pmpaddr40):
        case(csr_pmpaddr41):
        case(csr_pmpaddr42):
        case(csr_pmpaddr43):
        case(csr_pmpaddr44):
        case(csr_pmpaddr45):
        case(csr_pmpaddr46):
        case(csr_pmpaddr47):
        case(csr_pmpaddr48):
        case(csr_pmpaddr49):
        case(csr_pmpaddr50):
        case(csr_pmpaddr51):
        case(csr_pmpaddr52):
        case(csr_pmpaddr53):
        case(csr_pmpaddr54):
        case(csr_pmpaddr55):
        case(csr_pmpaddr56):
        case(csr_pmpaddr57):
        case(csr_pmpaddr58):
        case(csr_pmpaddr59):
        case(csr_pmpaddr60):
        case(csr_pmpaddr61):
        case(csr_pmpaddr62):
        case(csr_pmpaddr63):{
            ops.read = pmpaddr_read;
            ops.write = pmpaddr_write;
            ops.set = pmpaddr_set;
            ops.clear = pmpaddr_clear;
            break;
        }

        case csr_mcountinhibit: {
            ops.read = mcountinhibit_read;
            ops.write = mcountinhibit_write;
            ops.set = mcountinhibit_set;
            ops.clear = mcountinhibit_clear;
            break;
        }

        case csr_sstatus: {
            ops.read = sstatus_read;
            ops.write = sstatus_write;
            ops.set = sstatus_set;
            ops.clear = sstatus_clear;
            break;
        }
    
        case csr_sie: {
            ops.read = sie_read;
            ops.write = sie_write;
            ops.set = sie_set;
            ops.clear = sie_clear;
            break;
        }
    
        case csr_stvec: {
            ops.read = stvec_read;
            ops.write = stvec_write;
            ops.set = stvec_set;
            ops.clear = stvec_clear;
            break;
        }
    
        case csr_scounteren: {
            ops.read = scounteren_read;
            ops.write = scounteren_write;
            ops.set = scounteren_set;
            ops.clear = scounteren_clear;
            break;
        }
    
        case csr_senvcfg: {
            ops.read = senvcfg_read;
            ops.write = invalid_write;
            ops.set = senvcfg_set;
            ops.clear = senvcfg_clear;
            break;
        }
    
        case csr_sscratch: {
            ops.read = sscratch_read;
            ops.write = sscratch_write;
            ops.set = sscratch_set;
            ops.clear = sscratch_clear;
            break;
        }
    
        case csr_sepc: {
            ops.read = sepc_read;
            ops.write = sepc_write;
            ops.set = sepc_set;
            ops.clear = sepc_clear;
            break;
        }
    
        case csr_scause: {
            ops.read = scause_read;
            ops.write = scause_write;
            ops.set = scause_set;
            ops.clear = scause_clear;
            break;
        }
    
        case csr_stval: {
            ops.read = stval_read;
            ops.write = stval_write;
            ops.set = stval_set;
            ops.clear = stval_clear;
            break;
        }
    
        case csr_sip: {
            ops.read = sip_read;
            ops.write = sip_write;
            ops.set = sip_set;
            ops.clear = sip_clear;
            break;
        }
    
        case csr_satp: {
            ops.read = satp_read;
            ops.write = satp_write;
            ops.set = satp_set;
            ops.clear = satp_clear;
            break;
        }
    
        case csr_scontext: {
            ops.read = scontext_read;
            ops.write = scontext_write;
            ops.set = scontext_set;
            ops.clear = scontext_clear;
            break;
        }
    
        case csr_mvendorid: {
            ops.read = mvendorid_read;
            ops.write = invalid_write;
            ops.set = invalid_write;
            ops.clear = invalid_write;
            break;
        }
    
        case csr_marchid: {
            ops.read = marchid_read;
            ops.write = invalid_write;
            ops.set = invalid_write;
            ops.clear = invalid_write;
            break;
        }
    
        case csr_mimpid: {
            ops.read = mimpid_read;
            ops.write = invalid_write;
            ops.set = invalid_write;
            ops.clear = invalid_write;
            break;
        }
    
        case csr_mhartid: {
            ops.read = mhartid_read;
            ops.write = invalid_write;
            ops.set = invalid_write;
            ops.clear = invalid_write;
            break;
        }
    
        case csr_mconfigptr: {
            ops.read = mconfigptr_read;
            ops.write = mconfigptr_write;
            ops.set = mconfigptr_set;
            ops.clear = mconfigptr_clear;
            break;
        }
    
        case csr_mstatus: {
            ops.read = mstatus_read;
            ops.write = mstatus_write;
            ops.set = mstatus_set;
            ops.clear = mstatus_clear;
            break;
        }
    
        case csr_misa: {
            ops.read = misa_read;
            ops.write = misa_write;
            ops.set = misa_set;
            ops.clear = misa_clear;
            break;
        }
    
        case csr_medeleg: {
            ops.read = medeleg_read;
            ops.write = medeleg_write;
            ops.set = medeleg_set;
            ops.clear = medeleg_clear;
            break;
        }
    
        case csr_mideleg: {
            ops.read = mideleg_read;
            ops.write = mideleg_write;
            ops.set = mideleg_set;
            ops.clear = mideleg_clear;
            break;
        }
    
        case csr_mie: {
            ops.read = mie_read;
            ops.write = mie_write;
            ops.set = mie_set;
            ops.clear = mie_clear;
            break;
        }
    
        case csr_mtvec: {
            ops.read = mtvec_read;
            ops.write = mtvec_write;
            ops.set = mtvec_set;
            ops.clear = mtvec_clear;
            break;
        }
    
        case csr_mcounteren: {
            ops.read = mcounteren_read;
            ops.write = mcounteren_write;
            ops.set = mcounteren_set;
            ops.clear = mcounteren_clear;
            break;
        }
    
        case csr_mstatush: {
            ops.read = mstatush_read;
            ops.write = mstatush_write;
            ops.set = mstatush_set;
            ops.clear = mstatush_clear;
            break;
        }
    
        case csr_mscratch: {
            ops.read = mscratch_read;
            ops.write = mscratch_write;
            ops.set = mscratch_set;
            ops.clear = mscratch_clear;
            break;
        }
    
        case csr_mepc: {
            ops.read = mepc_read;
            ops.write = mepc_write;
            ops.set = mepc_set;
            ops.clear = mepc_clear;
            break;
        }
    
        case csr_mcause: {
            ops.read = mcause_read;
            ops.write = mcause_write;
            ops.set = mcause_set;
            ops.clear = mcause_clear;
            break;
        }
    
        case csr_mtval: {
            ops.read = mtval_read;
            ops.write = mtval_write;
            ops.set = mtval_set;
            ops.clear = mtval_clear;
            break;
        }
    
        case csr_mip: {
            ops.read = mip_read;
            ops.write = mip_write;
            ops.set = mip_set;
            ops.clear = mip_clear;
            break;
        }
    
        case csr_mtinst: {
            ops.read = mtinst_read;
            ops.write = mtinst_write;
            ops.set = mtinst_set;
            ops.clear = mtinst_clear;
            break;
        }
    
        case csr_mtval2: {
            ops.read = mtval2_read;
            ops.write = mtval2_write;
            ops.set = mtval2_set;
            ops.clear = mtval2_clear;
            break;
        }
    
        case csr_menvcfg: {
            ops.read = menvcfg_read;
            ops.write = menvcfg_write;
            ops.set = menvcfg_set;
            ops.clear = menvcfg_clear;
            break;
        }
    
        case csr_mevncfgh: {
            ops.read = mevncfgh_read;
            ops.write = mevncfgh_write;
            ops.set = mevncfgh_set;
            ops.clear = mevncfgh_clear;
            break;
        }
    
        case csr_mseccfg: {
            ops.read = mseccfg_read;
            ops.write = mseccfg_write;
            ops.set = mseccfg_set;
            ops.clear = mseccfg_clear;
            break;
        }
    
        case csr_mseccfgh: {
            ops.read = mseccfgh_read;
            ops.write = mseccfgh_write;
            ops.set = mseccfgh_set;
            ops.clear = mseccfgh_clear;
            break;
        }
    
        case csr_tselect: {
            ops.read = tselect_read;
            ops.write = tselect_write;
            ops.set = tselect_set;
            ops.clear = tselect_clear;
            break;
        }
    
        case csr_tdata1: {
            ops.read = tdata1_read;
            ops.write = tdata1_write;
            ops.set = tdata1_set;
            ops.clear = tdata1_clear;
            break;
        }
    
        case csr_tdata2: {
            ops.read = tdata2_read;
            ops.write = tdata2_write;
            ops.set = tdata2_set;
            ops.clear = tdata2_clear;
            break;
        }
    
        case csr_tdata3: {
            ops.read = tdata3_read;
            ops.write = tdata3_write;
            ops.set = tdata3_set;
            ops.clear = tdata3_clear;
            break;
        }
    
        case csr_mcontext: {
            ops.read = mcontext_read;
            ops.write = mcontext_write;
            ops.set = mcontext_set;
            ops.clear = mcontext_clear;
            break;
        }
    
        case csr_dcsr: {
            ops.read = dcsr_read;
            ops.write = dcsr_write;
            ops.set = dcsr_set;
            ops.clear = dcsr_clear;
            break;
        }
    
        case csr_dpc: {
            ops.read = dpc_read;
            ops.write = dpc_write;
            ops.set = dpc_set;
            ops.clear = dpc_clear;
            break;
        }
    
        case csr_dscratch0: {
            ops.read = dscratch0_read;
            ops.write = dscratch0_write;
            ops.set = dscratch0_set;
            ops.clear = dscratch0_clear;
            break;
        }
    
        case csr_dscratch1: {
            ops.read = dscratch1_read;
            ops.write = dscratch1_write;
            ops.set = dscratch1_set;
            ops.clear = dscratch1_clear;
            break;
        }
    }

    return ops;
}

rv_exc_t rv_csr_rw(rv_cpu_t* cpu, int csr, uint32_t value, uint32_t* read_target, bool read){

    csr_ops_t ops = get_csr_ops(csr);
    rv_exc_t ex = rv_exc_none;
    uint32_t temp_read_target = 0;

    if(read){
        ex = ops.read(cpu, csr, &temp_read_target);
    }

    if(ex == rv_exc_none){
        ex = ops.write(cpu, csr, value);
    }

    if(ex == rv_exc_none){
        *read_target = temp_read_target;
    }

    return ex;
}
rv_exc_t rv_csr_rs(rv_cpu_t* cpu, int csr, uint32_t value, uint32_t* read_target, bool write){
    csr_ops_t ops = get_csr_ops(csr);
    
    uint32_t temp_read_target = 0;

    rv_exc_t ex = ops.read(cpu, csr, &temp_read_target);

    if(ex == rv_exc_none && write){
        ex = ops.set(cpu, csr, value);
    }

    if(ex == rv_exc_none){
        *read_target = temp_read_target;
    }

    return ex;
}
rv_exc_t rv_csr_rc(rv_cpu_t* cpu, int csr, uint32_t value, uint32_t* read_target, bool write){
   
    csr_ops_t ops = get_csr_ops(csr);
    uint32_t temp_read_target = 0;
    rv_exc_t ex = ops.read(cpu, csr, &temp_read_target);
    
    if(ex == rv_exc_none && write){
        ex = ops.clear(cpu, csr, value);
    }

    if(ex == rv_exc_none){
        *read_target = temp_read_target;
    }

    return ex;
}

rv_priv_mode_t rv_csr_min_priv_mode(int csr) {
    int priv_num = ((csr >> 28) && 0b11);
    switch(priv_num){
        case 0b11:
            return rv_mmode;
        case 0b01:
            return rv_smode;
        case 0b00:
            return rv_umode;
    }
    return (rv_priv_mode_t)-1;
}