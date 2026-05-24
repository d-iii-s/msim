from base import *

adp = Adapter(int(sys.argv[1]))

# Check that write does not modify other CPU's registers
adp.send(ReadGeneralRegisterRequest, arg0=CPU1, arg1=1).expect_response(StatusOk, arg0=0x00)  # x1 (ra) is 0 at reset
adp.send(WriteGeneralRegisterRequest, arg0=CPU0, arg1=1, arg2=0x12345678).expect_response(StatusOk)
adp.send(ReadGeneralRegisterRequest, arg0=CPU0, arg1=1).expect_response(StatusOk, arg0=0x12345678)
adp.send(ReadGeneralRegisterRequest, arg0=CPU1, arg1=1).expect_response(StatusOk, arg0=0x00)

# Invalid register index
for reg in (REG_COUNT, REG_COUNT + 4546, REG_COUNT + 144104):
    adp.send(ReadGeneralRegisterRequest, arg0=CPU0, arg1=reg).expect_response(StatusRegisterNotFoundError, arg0=reg)
    adp.send(ReadGeneralRegisterRequest, arg0=CPU1, arg1=reg).expect_response(StatusRegisterNotFoundError, arg0=reg)

# Write and read all registers with a known pattern
for reg in range(1, REG_COUNT):
    val0, val1 = reg * 0x01234567, reg * 0x0123456789abcdef
    adp.send(WriteGeneralRegisterRequest, arg0=CPU0, arg1=reg, arg2=val0).expect_response(StatusOk)
    adp.send(ReadGeneralRegisterRequest, arg0=CPU0, arg1=reg).expect_response(StatusOk, arg0=val0)
    adp.send(WriteGeneralRegisterRequest, arg0=CPU1, arg1=reg, arg2=val1).expect_response(StatusOk)
    adp.send(ReadGeneralRegisterRequest, arg0=CPU1, arg1=reg).expect_response(StatusOk, arg0=val1)

# Write all registers with a different pattern
for reg in range(1, REG_COUNT):
    adp.send(WriteGeneralRegisterRequest, arg0=CPU0, arg1=reg, arg2=reg * 0x07654321).expect_response(StatusOk)
    adp.send(WriteGeneralRegisterRequest, arg0=CPU1, arg1=reg, arg2=reg * 0x00edcba987654321).expect_response(StatusOk)

# Step a bit and check that the PC is updated, but registers are unchanged
adp.send(StepRequest, arg0=CPU0, arg1=2).expect_response(StatusOk)
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=CPU0, arg1=at(2), arg2=StoppedReasonStep)
adp.send(ReadPCRequest, arg0=CPU0).expect_response(StatusOk, arg0=at(2))
adp.send(ReadPCRequest, arg0=CPU1).expect_response(StatusOk, arg0=at(2))

# Check that registers still have the same value after stepping
for reg in range(1, REG_COUNT):
    adp.send(ReadGeneralRegisterRequest, arg0=CPU0, arg1=reg).expect_response(StatusOk, arg0=reg * 0x07654321)
    adp.send(ReadGeneralRegisterRequest, arg0=CPU1, arg1=reg).expect_response(StatusOk, arg0=reg * 0x00edcba987654321)

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()