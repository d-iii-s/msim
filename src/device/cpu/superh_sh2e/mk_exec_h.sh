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

#ifndef SUPERH_SH2E_EXEC_H_
#define SUPERH_SH2E_EXEC_H_

#include "cpu.h"
#include "insn.h"


// Instruction execution functions

EOT

sed -nE '
	/^sh2e_insn_exec/ {
		s/^/extern sh2e_exception_t /
		s/const restrict //
		s/const insn/insn/
		s/ [{]$/;/
		p
	}
' exec.c

cat <<- EOT

#endif // SUPERH_SH2E_EXEC_H_
EOT
} | clang-format
