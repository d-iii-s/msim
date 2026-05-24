from base import *

adp = Adapter(int(sys.argv[1]))

nop = struct.pack("<I", NOP_INSTR)  # NOP instruction
assert len(nop) == INSTR_LEN

nops_per_word = 8 // INSTR_LEN
nop_word = int.from_bytes(nop * nops_per_word, byteorder='little')

adp.send(ReadPhysMemoryRequest, arg0=at(0)).expect_response(StatusOk, arg0=nop_word, arg1=nop_word, arg2=nop_word)
adp.send(ReadVirtMemoryRequest, arg0=DEFAULT_CPU, arg1=at(0)).expect_response(StatusOk, arg0=nop_word, arg1=nop_word,
                                                                              arg2=nop_word)

adp.send(ReadPhysMemoryRequest, arg0=at(2)).expect_response(StatusOk, arg0=nop_word, arg1=nop_word, arg2=nop_word)
adp.send(ReadVirtMemoryRequest, arg0=DEFAULT_CPU, arg1=at(2)).expect_response(StatusOk, arg0=nop_word, arg1=nop_word,
                                                                              arg2=nop_word)

# Unaligned access should also work
nop_bytes = nop * (nops_per_word + 1)  # extra byte of room
shifted = nop_bytes[1:9]  # 8 bytes starting at offset 1
nop_word_shifted = int.from_bytes(shifted, byteorder='little')
adp.send(ReadPhysMemoryRequest, arg0=RST_VEC + 1).expect_response(StatusOk, arg0=nop_word_shifted,
                                                                  arg1=nop_word_shifted, arg2=nop_word_shifted)
adp.send(ReadVirtMemoryRequest, arg0=DEFAULT_CPU, arg1=RST_VEC + 1).expect_response(StatusOk, arg0=nop_word_shifted,
                                                                                    arg1=nop_word_shifted,
                                                                                    arg2=nop_word_shifted)

# Write and read beyond the program code, physical then read as virtual
START = at(PROGRAM_LEN * 2)
READ_SIZE = 64
for addr in range(START, START + READ_SIZE + 1, READ_SIZE):
    value = addr ^ 0x11223344
    adp.send(ReadPhysMemoryRequest, arg0=addr).expect_response(StatusOk, arg0=0x00, arg1=0x00, arg2=0x00)
    adp.send(WritePhysMemoryRequest, arg0=addr, arg1=value).expect_response()
    adp.send(ReadVirtMemoryRequest, arg0=DEFAULT_CPU, arg1=addr).expect_response(StatusOk, arg0=value, arg1=0x00,
                                                                                 arg2=0x00)

# Write another pattern, virtual then read as physical
for addr in range(START, START + READ_SIZE + 1, READ_SIZE):
    value = addr ^ 0x44332211
    # adp.send(WriteVirtMemoryRequest, arg0=DEFAULT_CPU, arg1=addr, arg2=value).expect_response()
    adp.send(WritePhysMemoryRequest, arg0=addr, arg1=value).expect_response()
    adp.send(ReadPhysMemoryRequest, arg0=addr).expect_response(StatusOk, arg0=value, arg1=0x00, arg2=0x00)

# Check address translation
adp.send(TranslateAddressRequest, arg0=DEFAULT_CPU, arg1=at(0)).expect_response(StatusOk, arg0=at(0))
adp.send(TranslateAddressRequest, arg0=DEFAULT_CPU, arg1=at(PROGRAM_LEN)).expect_response(StatusOk, arg0=at(PROGRAM_LEN))

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()
