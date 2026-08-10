{
cat <<- EOT
/*
 * Copyright (c) X-Y Z
 * All rights reserved.
 *
 * Distributed under the terms of GPL.
 *
 *
 * Hitachi/Renesas SuperH SH-2E microprocessor device (32-bit, FPU).
 *
 */

#ifndef SUPERH_SH2E_DISASM_H_
#define SUPERH_SH2E_DISASM_H_

#include "cpu.h"
#include "decode.h"
#include "insn.h"

#include "../../../utils.h"

#include <stdint.h>


// Instruction disassembly functions

EOT

sed -nE '
	/^sh2e_insn_desc_dump/ {
		s/^/extern void /;
		N; N; N; N;
		s/\n/ /g; s/ +/ /g;
		s/ ?([()]) ?/\1/g;
		s/[{]$/;/;
		s/const restrict //g;
		s/const (addr|insn)/\1/g;
		p
	}
' disasm.c \
| grep 'comments)'

cat <<- EOT

#endif // SUPERH_SH2E_DISASM_H_
EOT
} | clang-format