from base import *

adp = Adapter(int(sys.argv[1]))

adp.send(ReadGeneralRegisterRequest, arg0=DEFAULT_CPU, arg1=0).expect_response(StatusOk, arg0=0x00)  # x0 is always 0
# Writing 0 to x0 should succeed but have no effect
adp.send(WriteGeneralRegisterRequest, arg0=DEFAULT_CPU, arg1=0).expect_response(StatusOk)
adp.send(ReadGeneralRegisterRequest, arg0=DEFAULT_CPU, arg1=0).expect_response(StatusOk, arg0=0x00)

adp.send(ReadGeneralRegisterRequest, arg0=DEFAULT_CPU, arg1=1).expect_response(StatusOk,
                                                                               arg0=0x00)  # x1 (ra) is 0 at reset
adp.send(WriteGeneralRegisterRequest, arg0=DEFAULT_CPU, arg1=1, arg2=0x12345678).expect_response(StatusOk)
adp.send(ReadGeneralRegisterRequest, arg0=DEFAULT_CPU, arg1=1).expect_response(StatusOk, arg0=0x12345678)

# Invalid register index
for reg in range(REG_COUNT, REG_COUNT + 50, 10):
    adp.send(ReadGeneralRegisterRequest, arg0=DEFAULT_CPU, arg1=reg).expect_response(StatusRegisterNotFoundError,
                                                                                     arg0=reg)

# Write and read all registers with a known pattern
for reg in range(1, REG_COUNT):
    value = reg * 0x01234567
    adp.send(WriteGeneralRegisterRequest, arg0=DEFAULT_CPU, arg1=reg, arg2=value).expect_response(StatusOk)
    adp.send(ReadGeneralRegisterRequest, arg0=DEFAULT_CPU, arg1=reg).expect_response(StatusOk, arg0=value)

# Write all registers with a different pattern
for reg in range(1, REG_COUNT):
    value = reg * 0x07654321
    adp.send(WriteGeneralRegisterRequest, arg0=DEFAULT_CPU, arg1=reg, arg2=value).expect_response(StatusOk)

# Step a bit and check that the PC is updated, but registers are unchanged
adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=2).expect_response(StatusOk)
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(2), arg2=StoppedReasonStep)
adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=2).expect_response(StatusOk)
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(4), arg2=StoppedReasonStep)

# Check that registers still have the same value after stepping
for reg in range(1, REG_COUNT):
    value = reg * 0x07654321
    adp.send(ReadGeneralRegisterRequest, arg0=DEFAULT_CPU, arg1=reg).expect_response(StatusOk, arg0=value)

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()