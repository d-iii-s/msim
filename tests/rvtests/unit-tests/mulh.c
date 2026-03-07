#include <stdint.h>
#include <stdio.h>
#include <pcut/pcut.h>

#define XLEN ARCH

#include "../../../src/device/cpu/riscv_rv_ima/types.h"

PCUT_INIT

PCUT_TEST_SUITE(mulh);

PCUT_TEST(mulhuu)
{
#if XLEN == 32
    PCUT_ASSERT_INT_EQUALS(0x0C379AAA, uxlen_mulhuu(0x12345678, 0xABCDEF01));
#endif
#if XLEN == 64
    PCUT_ASSERT_INT_EQUALS(0x121FA00AD77D7422, uxlen_mulhuu(0x123456789ABCDEF0, 0xFEDCBA9876543210));
#endif
}

PCUT_TEST(mulhsu)
{
#if XLEN == 32
    PCUT_ASSERT_INT_EQUALS(0xF3C86555, uxlen_mulhsu(-305419896, 0xABCDEF01));
#endif
#if XLEN == 64
    PCUT_ASSERT_INT_EQUALS(0xEDE05FF528828BDD, uxlen_mulhsu(-1311768467463790320, 0xFEDCBA9876543210));
#endif
}

PCUT_TEST(mulhss)
{
#if XLEN == 32
    PCUT_ASSERT_INT_EQUALS(0xc379aa, uxlen_mulhss(-305419896, -180150000));
#endif
#if XLEN == 64
    PCUT_ASSERT_INT_EQUALS(0x121fa00ad77d742, uxlen_mulhss(-1311768467463790320, -1147797409030816545));
#endif
}

PCUT_EXPORT(mulh);
