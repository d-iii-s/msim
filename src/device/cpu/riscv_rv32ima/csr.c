#include <stdint.h>
#include "csr.h"

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

void init_csr(csr_t *csr, unsigned int procno){
    
    csr->misa = RV_ISA;
    csr->mvendorid = RV_VENDOR_ID;
    csr->marchid = RV_ARCH_ID;
    csr->mimpid = RV_IMPLEMENTATION_ID;
    csr->mhartid = procno;

    //TODO: rest
}