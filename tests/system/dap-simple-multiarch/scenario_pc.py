from base import *

adp = Adapter(int(sys.argv[1]))

# Advance the PC by one instruction and check that it has advanced correctly only for CPU0, not CPU1
adp.send(WritePCRequest, arg0=CPU0, arg1=at(1)).expect_response(StatusOk)
adp.send(ReadPCRequest, arg0=CPU0).expect_response(StatusOk, arg0=at(1))
adp.send(ReadPCRequest, arg0=CPU1).expect_response(StatusOk, arg0=at(0))

# Step once CPU1
adp.send(StepRequest, arg0=CPU1, arg1=4).expect_response(StatusOk)
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=CPU1, arg1=at(4), arg2=StoppedReasonStep)
adp.send(ReadPCRequest, arg0=CPU0).expect_response(StatusOk, arg0=at(5))
adp.send(ReadPCRequest, arg0=CPU1).expect_response(StatusOk, arg0=at(4))

# Rollback the PC
adp.send(WritePCRequest, arg0=CPU0, arg1=at(0)).expect_response(StatusOk)
adp.send(WritePCRequest, arg0=CPU1, arg1=at(2)).expect_response(StatusOk)

# Step once CPU0
adp.send(StepRequest, arg0=CPU0, arg1=2).expect_response(StatusOk)
adp.send(ResumeRequest).expect_response()
adp.expect_event(StoppedAtEvent, arg0=CPU0, arg1=at(2), arg2=StoppedReasonStep)
adp.send(ReadPCRequest, arg0=CPU0).expect_response(StatusOk, arg0=at(2))
adp.send(ReadPCRequest, arg0=CPU1).expect_response(StatusOk, arg0=at(4))

adp.send(TerminateRequest).expect_response().expect_event(TerminatedEvent)
adp.close()