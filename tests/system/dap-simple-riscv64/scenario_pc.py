from base import *

adp = Adapter(int(sys.argv[1]))

adp.send(ReadPCRequest, arg0=DEFAULT_CPU).expect_response(StatusOk, arg0=at(0))

# Advance the PC by one instruction and check that it has advanced correctly
adp.send(WritePCRequest, arg0=DEFAULT_CPU, arg1=at(1)).expect_response(StatusOk)
adp.send(ReadPCRequest, arg0=DEFAULT_CPU).expect_response(StatusOk, arg0=at(1))

# Step once and check that the PC has advanced by another instruction
adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=1).expect_response(StatusOk)
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(2), arg2=StoppedReasonStep)
adp.send(ReadPCRequest, arg0=DEFAULT_CPU).expect_response(StatusOk, arg0=at(2))

# Rollback the PC
adp.send(WritePCRequest, arg0=DEFAULT_CPU, arg1=at(0)).expect_response(StatusOk)
adp.send(ReadPCRequest, arg0=DEFAULT_CPU).expect_response(StatusOk, arg0=at(0))

# Step through the entire program and check that the PC advances correctly each time, then reset it back to the start
half = PROGRAM_LEN // 2
rest = PROGRAM_LEN - half
for _ in range(2):
    # Step through the first half of the program and check that the PC has advanced correctly
    adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=half).expect_response(StatusOk)
    adp.send(ResumeRequest).expect_response()
    adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(half), arg2=StoppedReasonStep)
    # Step through the rest of the program and check that the PC has advanced correctly
    adp.send(StepRequest, arg0=DEFAULT_CPU, arg1=rest).expect_response(StatusOk)
    adp.send(ResumeRequest).expect_response()
    adp.expect_event(StoppedAtEvent, arg0=DEFAULT_CPU, arg1=at(PROGRAM_LEN), arg2=StoppedReasonStep)
    # Rollback the PC
    adp.send(WritePCRequest, arg0=DEFAULT_CPU, arg1=at(0)).expect_response(StatusOk)

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()
