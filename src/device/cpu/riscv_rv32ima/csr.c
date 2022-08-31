#include <stdint.h>
#include "csr.h"
#include "cpu.h"

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

static rv_exc_t invalid_read(rv_cpu_t* cpu, int csr, uint32_t* target){
    return rv_exc_illegal_instruction;
}
static rv_exc_t invalid_write(rv_cpu_t* cpu, int csr, uint32_t target){
    return rv_exc_illegal_instruction;
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

    return ops;
}

rv_exc_t rv_csr_rw(rv_cpu_t* cpu, int csr, uint32_t value, uint32_t* read_target, bool read){

    csr_ops_t ops = get_csr_ops(csr);
    rv_exc_t ex = rv_exc_none;

    if(read){
        ex = ops.read(cpu, csr, read_target);
    }

    if(ex == rv_exc_none){
        ex = ops.write(cpu, csr, value);
    }

    return ex;
}
rv_exc_t rv_csr_rs(rv_cpu_t* cpu, int csr, uint32_t value, uint32_t* read_target, bool read){
    csr_ops_t ops = get_csr_ops(csr);
    rv_exc_t ex = rv_exc_none;

    if(read){
        ex = ops.read(cpu, csr, read_target);
    }

    if(ex == rv_exc_none){
        ex = ops.set(cpu, csr, value);
    }
    return ex;
}
rv_exc_t rv_csr_rc(rv_cpu_t* cpu, int csr, uint32_t value, uint32_t* read_target, bool read){
   
    csr_ops_t ops = get_csr_ops(csr);
    rv_exc_t ex = rv_exc_none;

    if(read){
        ex = ops.read(cpu, csr, read_target);
    }

    if(ex == rv_exc_none){
        ex = ops.clear(cpu, csr, value);
    }
    return ex;
}
