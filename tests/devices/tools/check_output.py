#!/usr/bin/env python3

# SPDX-License-Identifier: Apache-2.0
# Copyright 2019 Charles University


import sys

PREFIX_BLOCK_EXPECTED = '[EXPECTED BLOCK]: '
PREFIX_EXPECTED = '[EXPECTED]: '
PREFIX_ACTUAL = '[ ACTUAL ]: '

def print_error(fmt: str, *args, **kwargs):
    print(fmt.format(*args, **kwargs), file=sys.stderr)

def escape_string(s: str):
    """Encodes non-printable ASCII characters using hexadecimal escape codes."""
    return "".join(c if (32 <= ord(c) <= 126) else '\\x{0:02x}'.format(ord(c)) for c in s)

def report_mismatch(actual: str, actual_line: int, expected: str, expected_line: int):
    print_error('Mismatch on lines {} and {} ("{}" != "{}")',
                expected_line, actual_line,
                escape_string(expected), escape_string(actual))

def main():
    expected_block = []
    expected_block_line = 0
    expected_block_frozen = False

    expected = None
    expected_line = 0

    line_number = 0
    exit_code = 0

    for line in sys.stdin:
        line = line.rstrip()
        line_number = line_number + 1
        if line.startswith(PREFIX_BLOCK_EXPECTED):
            if expected_block_frozen:
                print_error('Actual block on line {} ended too early on {} ("{}" not matched).',
                            expected_block_line, line_number - 1, expected_block[0])
                expected_block = []
                exit_code = 1
            if not expected_block:
                expected_block_line = line_number
            expected_block.append(line[len(PREFIX_BLOCK_EXPECTED):])
            expected_block_frozen = False
            continue
        else:
            if expected_block:
                expected_block_frozen = True

        if expected_block:
            if line != expected_block[0]:
                report_mismatch(line, line_number, expected_block[0], expected_block_line)
                exit_code = 1
            expected_block = expected_block[1:]
            expected_block_line = expected_block_line + 1
            if not expected_block:
                expected_block_frozen = False
            continue

        if line.startswith(PREFIX_EXPECTED):
            if not expected is None:
                print_error('Missing actual value for expected value on line {} ({}).',
                            expected_line, expected)
                exit_code = 1
            expected = line[len(PREFIX_EXPECTED):]
            expected_line = line_number
        elif line.startswith(PREFIX_ACTUAL):
            actual = line[len(PREFIX_ACTUAL):]
            if expected is None:
                print_error('Missing expected value for actual value on line {} ({}).',
                            line_number, actual)
                exit_code = 1
                continue
            if actual != expected:
                report_mismatch(actual, line_number, expected, expected_line)
                exit_code = 1
            expected = None
        else:
            # Skip other lines
            pass

    if expected_block:
        print_error('Not enough actual lines for expected block at {} ("{}" not mached).',
                    expected_block_line, expected_block[0])

    return exit_code


if __name__ == "__main__":
    sys.exit(main())
