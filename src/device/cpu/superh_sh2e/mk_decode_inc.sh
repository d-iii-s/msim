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

#ifndef SUPERH_SH2E_DECODE_INC_
#define SUPERH_SH2E_DECODE_INC_


// Instruction decoding functions

EOT

sed -nE '
	/^(static +)?sh2e_insn_desc_/ { s/(static +)?sh2e_/extern sh2e_/; h };
	/^sh2e_insn_decode/ { x; G; s/\n/ /; s/const insn/insn/; s/ [{]$/;/; p }
' decode.c \
| grep decode_

cat <<- EOT

#endif // SUPERH_SH2E_DECODE_INC_
EOT
} | clang-format
